// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2015-2024 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#include "rich_documenthandler.h"
#include "asyncimageprovider.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KStandardShortcut>

#include <KColorSchemeManager>
#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QCryptographicHash>
#include <QCursor>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPalette>
#include <QQmlFile>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextList>
#include <QTextTable>

using namespace Qt::StringLiterals;

constexpr int textMargin = 20;

namespace
{

const QChar kLinkBoundaryChar(u'\u2060');

QColor normalTextColor()
{
    return KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NormalText).color();
}

QColor codeBlockBackgroundColor()
{
    return KColorScheme(QPalette::Active, KColorScheme::View).background(KColorScheme::AlternateBackground).color();
}

QString internalLinkUrlForName(const QString &noteName)
{
    if (noteName.isEmpty()) {
        return {};
    }
    QUrl url;
    url.setScheme(u"marknote"_s);
    url.setHost(u"note"_s);
    url.setPath(u'/' + noteName);
    return url.toString(QUrl::FullyEncoded);
}

QString internalLinkNameFromUrl(const QUrl &url)
{
    if (url.scheme() != "marknote"_L1 || url.host() != "note"_L1) {
        return {};
    }
    const QString path = url.path(QUrl::FullyDecoded);
    if (path.isEmpty() || path == "/"_L1) {
        return {};
    }
    return path.mid(1);
}
}

RichDocumentHandler::RichDocumentHandler(QObject *parent)
    : DocumentHandler(parent)
{
    m_document = nullptr;
    m_textArea = nullptr;
    m_blockMargin = 0;
    m_cursorPosition = -1;
    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_lastFontFamily = fontFamily();
    m_lastFontSize = fontSize();
    m_lastTextColor = textColor();
    m_lastAlignment = alignment();
    m_lastBold = bold();
    m_lastItalic = italic();
    m_lastStrikethrough = strikethrough();
    m_lastUnderline = underline();

    // If the clipboard changes, the canPaste state might also change
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::changed, this, &RichDocumentHandler::canPasteChanged);
}

bool RichDocumentHandler::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_textArea && event->type() == QEvent::KeyPress) {
        return !processKeyEvent(static_cast<QKeyEvent *>(event));
    }

    if (event->type() == QEvent::ApplicationPaletteChange) {
        parseDocument();
    }
    // activate only links covered by press and release, on release
    // matches the behavior of TextArea::linkActivated
    // we can't use that directly though as it prevents placing the cursor inside a link
    if (auto me = static_cast<QMouseEvent *>(event);
        object == m_textArea && event->type() == QEvent::MouseButtonPress && me->modifiers() == Qt::ControlModifier) {
        m_activeLink = m_document->textDocument()->documentLayout()->anchorAt(me->position());
    }
    if (auto me = static_cast<QMouseEvent *>(event);
        object == m_textArea && event->type() == QEvent::MouseButtonRelease && me->modifiers() == Qt::ControlModifier) {
        const auto link = m_document->textDocument()->documentLayout()->anchorAt(me->position());
        if (!link.isEmpty() && m_activeLink == link) {
            const QUrl url(link);
            if (url.scheme() == "marknote"_L1) {
                const QString noteName = internalLinkNameFromUrl(url);
                if (!noteName.isEmpty()) {
                    Q_EMIT internalLinkActivated(noteName);
                    return true;
                }
            }
            QDesktopServices::openUrl(url);
            return true;
        }
        m_activeLink.clear();
    }

    return false;
}

Qt::Alignment RichDocumentHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return Qt::AlignLeft;
    return textCursor().blockFormat().alignment();
}

void RichDocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    Q_EMIT alignmentChanged();
}

bool RichDocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void RichDocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT boldChanged();
}

bool RichDocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

void RichDocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT italicChanged();
}

bool RichDocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

void RichDocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

bool RichDocumentHandler::strikethrough() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontStrikeOut();
}

void RichDocumentHandler::setStrikethrough(bool strikethrough)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikethrough);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT strikethroughChanged();
}

static void fixupTable(QTextFrame *frame)
{
    if (!frame)
        return;

    for (auto child : frame->childFrames()) {
        if (auto table = dynamic_cast<QTextTable *>(child)) {
            QTextTableFormat tableFormat = table->format();

            tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
            tableFormat.setLeftMargin(1);
            tableFormat.setRightMargin(1);
            tableFormat.setBorder(1);

            const int columns = table->columns();
            QList<QTextLength> constraints;
            constraints.reserve(columns);
            const qreal percentage = 100.0 / columns;
            const QTextLength textlength(QTextLength::PercentageLength, percentage);

            for (int i = 0; i < columns; ++i) {
                constraints.append(textlength);
            }

            tableFormat.setColumnWidthConstraints(constraints);
            tableFormat.setAlignment(Qt::AlignLeft);
            tableFormat.setCellPadding(4);
            tableFormat.setBorderCollapse(true);
            tableFormat.setTopMargin(textMargin);

            table->setFormat(tableFormat);
        }
        fixupTable(child);
    }
}

void RichDocumentHandler::load(const QUrl &fileUrl)
{
    if (fileUrl == m_fileUrl)
        return;

    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();

    if (!QFile::exists(fileUrl.toLocalFile()))
        return;

    QFile file(fileUrl.toLocalFile());
    if (!file.open(QFile::ReadOnly))
        return;

    const QString rawContent = QString::fromUtf8(file.readAll());

    // O(N) Table cell padding without creating temporary strings or using regex
    QString tableProcessedContent;
    tableProcessedContent.reserve(rawContent.size() + rawContent.size() / 10);

    const auto lines = QStringView(rawContent).split(u'\n');
    for (const auto &line : lines) {
        if (line.trimmed().startsWith(u'|')) {
            bool lastWasPipe = false;
            int spaceCount = 0;

            for (qsizetype j = 0; j < line.size(); ++j) {
                const QChar c = line[j];
                if (c == u'|') {
                    if (lastWasPipe) {
                        tableProcessedContent.append(u"\u00A0|"_s);
                    } else {
                        tableProcessedContent.append(c);
                        lastWasPipe = true;
                    }
                    spaceCount = 0;
                } else if (c == u' ') {
                    if (lastWasPipe) {
                        spaceCount++;
                    } else {
                        tableProcessedContent.append(c);
                    }
                } else {
                    if (lastWasPipe) {
                        if (spaceCount > 0) {
                            tableProcessedContent.append(QString(spaceCount, u' '));
                            spaceCount = 0;
                        }
                        lastWasPipe = false;
                    }
                    tableProcessedContent.append(c);
                }
            }
            if (lastWasPipe && spaceCount > 0) {
                tableProcessedContent.append(QString(spaceCount, u' '));
            }
        } else {
            tableProcessedContent.append(line);
        }
        tableProcessedContent.append(u'\n');
    }

    if (!tableProcessedContent.isEmpty()) {
        tableProcessedContent.chop(1);
    }

    m_imagePathLookup.clear();

    // Unified Image and Wiki Link resolution using QStringBuilder (%)
    QString finalContent;
    finalContent.reserve(tableProcessedContent.size() + tableProcessedContent.size() / 5);

    static const QRegularExpression imgRegex(u"!\\[.*?\\]\\(([^)]+)\\)"_s, QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression wikiRegex(u"\\[\\[([^\\]\\n]+)\\]\\]"_s);

    int lastPos = 0;
    QRegularExpressionMatchIterator imgIt = imgRegex.globalMatch(tableProcessedContent);

    // We need baseUrl for images, grab it early if possible
    QUrl baseUrl = QUrl(fileUrl).adjusted(QUrl::RemoveFilename);
    if (QTextDocument *doc = textDocument()) {
        doc->setBaseUrl(baseUrl);
    }

    while (imgIt.hasNext()) {
        QRegularExpressionMatch match = imgIt.next();

        // 1. Get the chunk of text *before* the image
        QStringView textBeforeImage = QStringView(tableProcessedContent).mid(lastPos, match.capturedStart() - lastPos);

        // 2. Resolve Wiki Links inside that chunk, appending directly to the final buffer
        int wikiLastPos = 0;
        QRegularExpressionMatchIterator wikiIt = wikiRegex.globalMatchView(textBeforeImage);
        while (wikiIt.hasNext()) {
            QRegularExpressionMatch wikiMatch = wikiIt.next();
            finalContent.append(textBeforeImage.mid(wikiLastPos, wikiMatch.capturedStart() - wikiLastPos));

            const QString linkBody = wikiMatch.captured(1).trimmed();
            if (linkBody.isEmpty()) {
                finalContent.append(wikiMatch.capturedView());
                wikiLastPos = wikiMatch.capturedEnd();
                continue;
            }

            QString noteName = linkBody;
            QString alias;
            const int pipeIndex = linkBody.indexOf(u'|');
            if (pipeIndex != -1) {
                noteName = linkBody.left(pipeIndex).trimmed();
                alias = linkBody.mid(pipeIndex + 1).trimmed();
            }

            const QString url = internalLinkUrlForName(noteName);
            if (url.isEmpty()) {
                finalContent.append(wikiMatch.capturedView());
            } else {
                const QString linkText = alias.isEmpty() ? noteName : alias;
                finalContent.append(u"["_s % linkText % u"]("_s % url % u")"_s);
            }
            wikiLastPos = wikiMatch.capturedEnd();
        }
        finalContent.append(textBeforeImage.mid(wikiLastPos));

        // Process the Image itself
        QStringView originalPathView = match.capturedView(1).trimmed();
        int quoteIndex = originalPathView.indexOf(u" \"");
        if (quoteIndex == -1)
            quoteIndex = originalPathView.indexOf(u" '");
        if (quoteIndex != -1)
            originalPathView = originalPathView.left(quoteIndex).trimmed();

        if (originalPathView.startsWith(u'<') && originalPathView.endsWith(u'>')) {
            originalPathView = originalPathView.mid(1, originalPathView.length() - 2);
        }

        QUrl absoluteUrl = baseUrl.resolved(QUrl(originalPathView.toString()));
        QString proxyUrl = processImage(absoluteUrl);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        finalContent.append(u"<br /><img style=\"max-width: 100%\" src=\""_s % proxyUrl % u"\" /><br />"_s);
#else
        finalContent.append(u"<br /><img width=\"500\" src=\""_s % proxyUrl % u"\" /><br />"_s);
#endif
        lastPos = match.capturedEnd();
    }

    // 4. Resolve Wiki Links in the remaining trailing text
    QStringView trailingText = QStringView(tableProcessedContent).mid(lastPos);
    int wikiLastPos = 0;
    QRegularExpressionMatchIterator wikiIt = wikiRegex.globalMatchView(trailingText);
    while (wikiIt.hasNext()) {
        QRegularExpressionMatch wikiMatch = wikiIt.next();
        finalContent.append(trailingText.mid(wikiLastPos, wikiMatch.capturedStart() - wikiLastPos));

        const QString linkBody = wikiMatch.captured(1).trimmed();
        if (linkBody.isEmpty()) {
            finalContent.append(wikiMatch.capturedView());
            wikiLastPos = wikiMatch.capturedEnd();
            continue;
        }

        QString noteName = linkBody;
        QString alias;
        const int pipeIndex = linkBody.indexOf(u'|');
        if (pipeIndex != -1) {
            noteName = linkBody.left(pipeIndex).trimmed();
            alias = linkBody.mid(pipeIndex + 1).trimmed();
        }

        const QString url = internalLinkUrlForName(noteName);
        if (url.isEmpty()) {
            finalContent.append(wikiMatch.capturedView());
        } else {
            const QString linkText = alias.isEmpty() ? noteName : alias;
            finalContent.append(u"["_s % linkText % u"]("_s % url % u")"_s);
        }
        wikiLastPos = wikiMatch.capturedEnd();
    }
    finalContent.append(trailingText.mid(wikiLastPos));

    Q_EMIT loaded(finalContent, Qt::MarkdownText);

    // 2. NOW access the document, freeze the layout engine, and apply constraints
    if (QTextDocument *doc = textDocument()) {
        doc->setUndoRedoEnabled(false);

        QTextCursor editCursor(doc);
        editCursor.beginEditBlock();

        // 3. The tables now exist, so fixupTable will successfully apply 100% width
        fixupTable(doc->rootFrame());
        parseDocument();

        QTextCursor checkCursor(doc);
        checkCursor.movePosition(QTextCursor::End);

        if (checkCursor.blockFormat().headingLevel() > 0) {
            checkCursor.insertBlock();
            QTextBlockFormat bf;
            bf.setHeadingLevel(0);
            checkCursor.setBlockFormat(bf);

            QTextCharFormat resetCharFormat;
            resetCharFormat.setFontWeight(QFont::Normal);
            resetCharFormat.setProperty(QTextFormat::FontSizeAdjustment, 0);

            checkCursor.setBlockCharFormat(resetCharFormat);
            checkCursor.setCharFormat(resetCharFormat);
        }

        editCursor.endEditBlock();

        doc->setModified(false);
        doc->clearUndoRedoStacks();
        doc->setUndoRedoEnabled(true);
    } else {
        // Fallback if the UI hasn't initialized the document
        Q_EMIT loaded(finalContent, Qt::MarkdownText);
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    moveCursor(cursor.position());
    reset();
}

void RichDocumentHandler::saveAs(const QUrl &fileUrl)
{
    QTextDocument *doc = textDocument();

    if (!doc || !doc->isModified()) {
        if (fileUrl != m_fileUrl) {
            m_fileUrl = fileUrl;
            Q_EMIT fileUrlChanged();
        }
        return;
    }

    const QString markdown = doc->toMarkdown(QTextDocument::MarkdownDialectGitHub);

    // Compile regexes once in memory
    static const QRegularExpression separatorRowRegex(u"^[\\|\\-\\:\\s]+$"_s);
    static const QRegularExpression dashesRegex(u"-+"_s);
    static const QRegularExpression linkRegex(u"\\]\\(image://marknote/([a-f0-9]+)[^)]*\\)"_s);
    static const QRegularExpression internalMarkdownRegex(u"\\[([^\\]]+)\\]\\((marknote:[^)]+)\\)"_s);

    // Use QStringView to prevent heap allocations
    QString processedMarkdown;
    processedMarkdown.reserve(markdown.size()); // Pre-allocate exact memory needed

    bool inCodeBlock = false;
    bool extraLineDeleted = false;

    const auto lines = QStringView(markdown).split(u'\n');
    for (const auto &line : lines) {
        if (inCodeBlock) {
            if (line.isEmpty()) {
                if (!extraLineDeleted) {
                    extraLineDeleted = true;
                    continue;
                }
            } else {
                extraLineDeleted = false;
            }
        }

        if (line.trimmed().startsWith(u"```"_s)) {
            inCodeBlock = !inCodeBlock;
            processedMarkdown.append(line);
        } else if (line.trimmed().startsWith(u'|')) {
            QString tableLine = line.toString();
            tableLine.replace(u"\u00A0"_s, u""_s);
            tableLine.replace(u"&nbsp;"_s, u""_s);

            // Fast O(N) manual pass instead of expensive negative lookahead regexes
            QString optimizedTableLine;
            optimizedTableLine.reserve(tableLine.size());
            bool escaped = false;
            for (int j = 0; j < tableLine.size(); ++j) {
                const QChar c = tableLine[j];
                if (c == u'\\') {
                    escaped = !escaped;
                    optimizedTableLine.append(c);
                } else if (c == u'|') {
                    // Remove trailing spaces before this pipe
                    while (optimizedTableLine.endsWith(u' ')) {
                        optimizedTableLine.chop(1);
                    }
                    optimizedTableLine.append(c);
                    // Skip leading spaces after this pipe
                    if (!escaped) {
                        while (j + 1 < tableLine.size() && tableLine[j + 1] == u' ') {
                            j++;
                        }
                    }
                    escaped = false;
                } else {
                    optimizedTableLine.append(c);
                    escaped = false;
                }
            }

            if (separatorRowRegex.match(optimizedTableLine).hasMatch()) {
                optimizedTableLine.replace(dashesRegex, u"-"_s);
            }
            processedMarkdown.append(optimizedTableLine);
        } else {
            // No allocation, just appends the view directly to the output buffer
            processedMarkdown.append(line);
        }
        processedMarkdown.append(u'\n');
    }

    if (!processedMarkdown.isEmpty()) {
        processedMarkdown.chop(1); // Remove trailing newline
    }

    // Unified Image and Wiki Link resolution using QStringBuilder (%)
    QString finalOutput;
    finalOutput.reserve(processedMarkdown.size());

    QRegularExpressionMatchIterator imgIt = linkRegex.globalMatch(processedMarkdown);
    int lastPos = 0;

    while (imgIt.hasNext()) {
        QRegularExpressionMatch imgMatch = imgIt.next();

        // 1. Process chunk before image for internal wiki links
        QStringView textBeforeImage = QStringView(processedMarkdown).mid(lastPos, imgMatch.capturedStart() - lastPos);
        int internalLastPos = 0;
        QRegularExpressionMatchIterator internalIt = internalMarkdownRegex.globalMatchView(textBeforeImage);

        while (internalIt.hasNext()) {
            QRegularExpressionMatch intMatch = internalIt.next();
            finalOutput.append(textBeforeImage.mid(internalLastPos, intMatch.capturedStart() - internalLastPos));

            const QString linkText = intMatch.captured(1);
            const QUrl url(intMatch.captured(2));
            const QString noteName = internalLinkNameFromUrl(url);

            if (noteName.isEmpty()) {
                finalOutput.append(intMatch.capturedView());
            } else if (linkText == noteName) {
                finalOutput.append(u"[["_s % noteName % u"]]"_s);
            } else {
                finalOutput.append(u"[["_s % noteName % u"|"_s % linkText % u"]]"_s);
            }
            internalLastPos = intMatch.capturedEnd();
        }
        finalOutput.append(textBeforeImage.mid(internalLastPos));

        // Process Image Link
        QString hash = imgMatch.captured(1);
        if (m_imagePathLookup.contains(hash)) {
            finalOutput.append(u"]("_s % m_imagePathLookup.value(hash) % u")"_s);
        } else {
            finalOutput.append(imgMatch.capturedView());
        }
        lastPos = imgMatch.capturedEnd();
    }

    // Process remaining trailing text for internal wiki links
    QStringView trailingText = QStringView(processedMarkdown).mid(lastPos);
    int internalLastPos = 0;
    QRegularExpressionMatchIterator internalIt = internalMarkdownRegex.globalMatchView(trailingText);

    while (internalIt.hasNext()) {
        QRegularExpressionMatch intMatch = internalIt.next();
        finalOutput.append(trailingText.mid(internalLastPos, intMatch.capturedStart() - internalLastPos));

        const QString linkText = intMatch.captured(1);
        const QUrl url(intMatch.captured(2));
        const QString noteName = internalLinkNameFromUrl(url);

        if (noteName.isEmpty()) {
            finalOutput.append(intMatch.capturedView());
        } else if (linkText == noteName) {
            finalOutput.append(u"[["_s % noteName % u"]]"_s);
        } else {
            finalOutput.append(u"[["_s % noteName % u"|"_s % linkText % u"]]"_s);
        }
        internalLastPos = intMatch.capturedEnd();
    }
    finalOutput.append(trailingText.mid(internalLastPos));

    // Strip out the boundary character in one highly optimized native C-level pass
    finalOutput.remove(kLinkBoundaryChar);

    // Check existence without reading the whole file into memory
    QFileInfo fileCheck(fileUrl.toLocalFile());
    if (fileCheck.exists()) {
        QFile file(fileUrl.toLocalFile());
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            Q_EMIT error(tr("Cannot save: ") + file.errorString() + u' ' + fileUrl.toLocalFile());
            return;
        }

        file.write(finalOutput.toUtf8());
        file.close();

        if (fileUrl != m_fileUrl) {
            m_fileUrl = fileUrl;
            Q_EMIT fileUrlChanged();
        }

        // Reset modified state so the next debouncer ticks hit the Early Bailout
        doc->setModified(false);
    }
}

void RichDocumentHandler::reset()
{
    if (fontFamily() != m_lastFontFamily) {
        Q_EMIT fontFamilyChanged();
    }
    if (alignment() != m_lastAlignment) {
        Q_EMIT alignmentChanged();
    }
    if (bold() != m_lastBold) {
        Q_EMIT boldChanged();
    }
    if (italic() != m_lastItalic) {
        Q_EMIT italicChanged();
    }
    if (underline() != m_lastUnderline) {
        Q_EMIT underlineChanged();
    }
    if (strikethrough() != m_lastStrikethrough) {
        Q_EMIT strikethroughChanged();
    }
    if (fontSize() != m_lastFontSize) {
        Q_EMIT fontSizeChanged();
    }
    if (textColor() != m_lastTextColor) {
        Q_EMIT textColorChanged();
    }

    m_lastFontFamily = fontFamily();
    m_lastAlignment = alignment();
    m_lastBold = bold();
    m_lastItalic = italic();
    m_lastUnderline = underline();
    m_lastStrikethrough = strikethrough();
    m_lastFontSize = fontSize();
    m_lastTextColor = textColor();
}

bool RichDocumentHandler::canIndentList() const
{
    return m_nestedListHelper.canIndent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

bool RichDocumentHandler::canDedentList() const
{
    return m_nestedListHelper.canDedent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

int RichDocumentHandler::currentListStyle() const
{
    if (!textCursor().currentList()) {
        return 0;
    }

    return -textCursor().currentList()->format().style();
}

int RichDocumentHandler::currentHeadingLevel() const
{
    return textCursor().blockFormat().headingLevel();
}

void RichDocumentHandler::indentListMore()
{
    m_nestedListHelper.handleOnIndentMore(textCursor());
}

void RichDocumentHandler::indentListLess()
{
    m_nestedListHelper.handleOnIndentLess(textCursor());
}

void RichDocumentHandler::setListStyle(int styleIndex)
{
    m_nestedListHelper.handleOnBulletType(-styleIndex, textCursor());
}

void RichDocumentHandler::setHeadingLevel(int level)
{
    const int boundedLevel = qBound(0, level, 6);

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blkfmt = cursor.blockFormat();
    blkfmt.setHeadingLevel(boundedLevel);

    // Apply margins dynamically when changing heading levels
    if (boundedLevel > 0) {
        blkfmt.setTopMargin(m_blockMargin * 2);
        blkfmt.setBottomMargin(m_blockMargin);
    } else {
        blkfmt.setTopMargin(m_blockMargin);
        blkfmt.setBottomMargin(0);
    }

    cursor.setBlockFormat(blkfmt);

    // FontSizeAdjustment goes from 3 for Heading 1 to -2 for Heading 6
    const int fontSizeAdjustment = 4 - boundedLevel;

    QTextCharFormat chrfmt = cursor.charFormat();
    chrfmt.setFontWeight(boundedLevel > 0 ? QFont::Bold : QFont::Normal);

    if (boundedLevel > 0) {
        chrfmt.setProperty(QTextFormat::FontSizeAdjustment, fontSizeAdjustment);
    } else {
        chrfmt.clearProperty(QTextFormat::FontSizeAdjustment);
        chrfmt.setFontPointSize(QFontInfo(textDocument()->defaultFont()).pointSize());
    }

    // Applying style to the current line or selection
    QTextCursor selectCursor = cursor;
    if (selectCursor.hasSelection()) {
        QTextCursor top = selectCursor;
        top.setPosition(qMin(top.anchor(), top.position()));
        top.movePosition(QTextCursor::StartOfBlock);

        QTextCursor bottom = selectCursor;
        bottom.setPosition(qMax(bottom.anchor(), bottom.position()));
        bottom.movePosition(QTextCursor::EndOfBlock);

        selectCursor.setPosition(top.position(), QTextCursor::MoveAnchor);
        selectCursor.setPosition(bottom.position(), QTextCursor::KeepAnchor);
        selectCursor.setCharFormat(chrfmt);
    } else {
        selectCursor.select(QTextCursor::BlockUnderCursor);
        cursor.setBlockCharFormat(chrfmt);
    }

    cursor.endEditBlock();
}

QString RichDocumentHandler::currentLinkUrl() const
{
    return textCursor().charFormat().anchorHref();
}

QString RichDocumentHandler::currentLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

void RichDocumentHandler::selectLinkText(QTextCursor *cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor->charFormat().isAnchor()) {
        const QString aHref = cursor->charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atStart()) {
                break;
            }
            cursor->setPosition(cursor->position() - 1);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }

        // Move selection to the end of the link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atEnd()) {
                break;
            }
            const int oldPosition = cursor->position();
            cursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            // Wordaround Qt Bug. when we have a table.
            // FIXME selection url
            if (oldPosition == cursor->position()) {
                break;
            }
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() - 1, QTextCursor::KeepAnchor);
        }
    } else if (cursor->hasSelection()) {
        // Nothing to do. Using the currently selected text as the link text.
    } else {
        // Select current word
        cursor->movePosition(QTextCursor::StartOfWord);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

void RichDocumentHandler::updateLink(const QString &linkUrl, const QString &linkText)
{
    auto cursor = textCursor();
    selectLinkText(&cursor);

    cursor.beginEditBlock();

    QTextDocument defaultTextDocument;
    const QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();
    QTextCharFormat trailingFormat = cursor.charFormat();
    trailingFormat.setAnchor(false);
    trailingFormat.setAnchorHref(QString());
    trailingFormat.setUnderlineStyle(defaultCharFormat.underlineStyle());
    trailingFormat.setUnderlineColor(defaultCharFormat.underlineColor());
    trailingFormat.setForeground(normalTextColor());

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format = cursor.charFormat();
    // Save original format to create an extra space with the existing char
    // format for the block
    if (!linkUrl.isEmpty()) {
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::NoUnderline);
        format.setForeground(linkColor());
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        format.setUnderlineStyle(defaultCharFormat.underlineStyle());
        format.setUnderlineColor(defaultCharFormat.underlineColor());
        format.setForeground(defaultCharFormat.foreground());
    }

    // Insert link text specified in dialog, otherwise write out url.
    QString _linkText;
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    cursor.insertText(_linkText, format);
    cursor.insertText(QString(kLinkBoundaryChar), trailingFormat);

    cursor.endEditBlock();
}

QColor RichDocumentHandler::linkColor()
{
    if (mLinkColor.isValid()) {
        return mLinkColor;
    }
    regenerateColorScheme();
    return mLinkColor;
}

void RichDocumentHandler::regenerateColorScheme()
{
    mLinkColor = QGuiApplication::palette().color(QPalette::Link);
    // TODO update existing link
}

QString RichDocumentHandler::processImage(const QUrl &originalUrl)
{
    if (auto engine = qmlEngine(this)) {
        if (!engine->imageProvider(u"marknote"_s)) {
            engine->addImageProvider(u"marknote"_s, new AsyncImageProvider);
        }
    }

    if (!originalUrl.isLocalFile())
        return originalUrl.toString();

    const QByteArray hash = QCryptographicHash::hash(originalUrl.toEncoded(), QCryptographicHash::Md5).toHex();
    const QString key = QString::fromLatin1(hash);
    const QString providerUrl = u"image://marknote/"_s + key;

    // Register Path for the Provider & SaveAs
    // PERFORMANCE WIN: We do NOT load the image here anymore.
    // We just tell the registry where it is. The Provider loads it in the
    // background.
    {
        AsyncImageProvider::registerPath(key, originalUrl.toLocalFile());
        s_pathRegistry[key] = originalUrl.toLocalFile();
    }

    // Also keep local lookup for SaveAs logic (redundant but keeps class logic
    // clean)
    m_imagePathLookup[key] = originalUrl.toLocalFile();

    return providerUrl;
}

void RichDocumentHandler::insertImage(const QUrl &url)
{
    if (!url.isLocalFile())
        return;

    QMimeDatabase db;
    const QMimeType mimeType = db.mimeTypeForFile(url.toLocalFile());

    if (!mimeType.name().startsWith(u"image/"_s)) {
        qWarning() << "Ignored non-image file:" << url.toLocalFile();
        return;
    }
    const QString proxyUrl = processImage(url);

    QTextCursor cursor = textCursor();

    cursor.insertHtml(u"<br />"_s);

    while (canDedentList()) {
        m_nestedListHelper.handleOnIndentLess(cursor);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    const QString html = u"<img style=\"max-width: 100%\" src=\""_s + proxyUrl + u"\" />"_s;
#else
    const QString html = u"<img width=\"500\" src=\""_s + proxyUrl + u"\" />"_s;
#endif

    cursor.insertHtml(html);
    cursor.insertHtml(u"<br />"_s);
}

void RichDocumentHandler::insertTable(int rows, int columns)
{
    QTextCursor cursor = textCursor();
    QTextTableFormat tableFormat;

    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setLeftMargin(1);
    tableFormat.setRightMargin(1);
    tableFormat.setBorder(1);

    QList<QTextLength> constraints;
    constraints.reserve(columns);
    const qreal percentage = 100.0 / columns;
    const QTextLength textlength(QTextLength::PercentageLength, percentage);

    for (int i = 0; i < columns; ++i) {
        constraints.append(textlength);
    }
    tableFormat.setColumnWidthConstraints(constraints);

    tableFormat.setAlignment(Qt::AlignLeft);
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(4);
    tableFormat.setBorderCollapse(true);
    tableFormat.setTopMargin(20);

    Q_ASSERT(cursor.document());

    // Create the table (Qt will initially see empty cells)
    QTextTable *table = cursor.insertTable(rows, columns, tableFormat);

    // Inflate the cells with protective shield space
    for (int i = 0; i < table->rows(); ++i) {
        for (int j = 0; j < table->columns(); ++j) {
            QTextTableCell cell = table->cellAt(i, j);
            if (cell.isValid()) {
                // Use Non-Breaking Space
                cell.firstCursorPosition().insertText(u"\u00A0"_s);
            }
        }
    }
    table->setFormat(tableFormat);
}

bool RichDocumentHandler::canPaste() const
{
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    return mimeData && (mimeData->hasHtml() || mimeData->hasText() || mimeData->hasFormat(QStringLiteral("text/markdown")) || mimeData->hasImage());
}

void RichDocumentHandler::pasteFromClipboard()
{
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    if (!mimeData) {
        return;
    }

    if (mimeData->hasUrls()) {
        bool pastedImage = false;
        const QList<QUrl> urls = mimeData->urls();

        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QMimeDatabase db;
                const QMimeType mimeType = db.mimeTypeForFile(url.toLocalFile());

                if (mimeType.name().startsWith(u"image/"_s)) {
                    insertImage(url);
                    pastedImage = true;
                }
            }
        }

        // If we successfully intercepted and pasted image(s), stop here
        // so we don't duplicate them as raw text strings.
        if (pastedImage) {
            return;
        }
    }

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    if (isCodeBlock(cursor.block())) {
        // we want to paste everything as plain text inside a code block,
        // except images which will be pasted normally in a new block
        cursor.insertText(mimeData->text());
        cursor.endEditBlock();
        parseDocument();
        return;
    }

    if (mimeData->hasHtml()) {
        cursor.insertHtml(mimeData->html());
    } else if (mimeData->hasFormat(QStringLiteral("text/markdown"))) {
        const QByteArray md = mimeData->data(QStringLiteral("text/markdown"));
        cursor.insertText(QString::fromUtf8(md));
    } else if (mimeData->hasText()) {
        cursor.insertText(mimeData->text());
    } else if (mimeData->hasImage()) {
        const auto currentDirectory = property("fileUrl").toUrl().adjusted(QUrl::RemoveFilename);
        const auto image = qvariant_cast<QImage>(mimeData->imageData());
        QDir dir(currentDirectory.toLocalFile());
        const auto newFileName = dir.filePath(QString::number(QDateTime::currentMSecsSinceEpoch()) + u".png"_s);

        if (image.save(newFileName)) {
            insertImage(QUrl::fromLocalFile(newFileName));
        } else {
            Q_EMIT showToast(i18n("Failed to save pasted image"));
        }
    }

    cursor.endEditBlock();

    parseDocument();
}

void RichDocumentHandler::setCheckable(bool add)
{
    QTextBlockFormat fmt;
    fmt.setMarker(add ? QTextBlockFormat::MarkerType::Unchecked : QTextBlockFormat::MarkerType::NoMarker);
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    if (add && !cursor.currentList()) {
        // Checkbox only works with lists, so if we are not at list, add a new one
        setListStyle(1);
    } else if (!add && cursor.currentList() && cursor.currentList()->count() == 1) {
        // If this is a single-element list with a checkbox, and user disables
        // a checkbox, assume user don't want a list too
        // (so when cursor is not on a list, and enables checkbox and disables
        // it right after, he returns to the same state with no list)
        setListStyle(0);
    }
    cursor.mergeBlockFormat(fmt);
    cursor.endEditBlock();

    Q_EMIT checkableChanged();
}

bool RichDocumentHandler::checkable() const
{
    return textCursor().blockFormat().marker() == QTextBlockFormat::MarkerType::Unchecked
        || textCursor().blockFormat().marker() == QTextBlockFormat::MarkerType::Checked;
}

bool RichDocumentHandler::evaluateReturnKeySupport(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Return) {
        return evaluateListSupport(event);
    }

    QTextCursor cursor = textCursor();

    int quoteLevel = cursor.blockFormat().intProperty(QTextFormat::BlockQuoteLevel);
    if (quoteLevel > 0) {
        // If pressing Enter on an empty line (unless it's not a list), exit the blockquote
        if (cursor.block().text().trimmed().isEmpty() && !cursor.currentList()) {
            cursor.beginEditBlock();
            QTextBlockFormat bfmt = cursor.blockFormat();

            // Decrease the level by 1 to gracefully exit nested quotes
            int newLevel = qMax(0, quoteLevel - 1);
            bfmt.setProperty(QTextFormat::BlockQuoteLevel, newLevel);
            bfmt.setIndent(newLevel);
            cursor.setBlockFormat(bfmt);

            cursor.endEditBlock();

            // Eat the event so we just drop the formatting without adding a new line
            return false;
        }

        // For non-empty lines, just let the list handler and native Qt layout handle it
        return evaluateListSupport(event);
    }

    const int oldPos = cursor.position();
    const int blockPos = cursor.block().position();

    // selection all the line.
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString lineText = cursor.selectedText();
    if (((oldPos - blockPos) > 0) && ((oldPos - blockPos) < int(lineText.length()))) {
        bool isQuotedLine = false;
        int bot = 0; // bot = begin of text after quote indicators
        while (bot < lineText.length()) {
            if ((lineText[bot] == QChar::fromLatin1('>')) || (lineText[bot] == QChar::fromLatin1('|'))) {
                isQuotedLine = true;
                ++bot;
            } else if (lineText[bot].isSpace()) {
                ++bot;
            } else {
                break;
            }
        }
        evaluateListSupport(event);
        // duplicate quote indicators of the previous line before the new
        // line if the line actually contained text (apart from the quote
        // indicators) and the cursor is behind the quote indicators
        if (isQuotedLine && (bot != lineText.length()) && ((oldPos - blockPos) >= int(bot))) {
            // The cursor position might have changed unpredictably if there was selected
            // text which got replaced by a new line, so we query it again:
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QString newLine = cursor.selectedText();

            // remove leading white space from the new line and instead
            // add the quote indicators of the previous line
            int leadingWhiteSpaceCount = 0;
            while ((leadingWhiteSpaceCount < newLine.length()) && newLine[leadingWhiteSpaceCount].isSpace()) {
                ++leadingWhiteSpaceCount;
            }
            newLine.replace(0, leadingWhiteSpaceCount, lineText.left(bot));
            cursor.insertText(newLine);
            // cursor.setPosition( cursor.position() + 2 );
            cursor.movePosition(QTextCursor::StartOfBlock);
            setCursorPosition(cursor.position());
        }
        return true;
    } else {
        return evaluateListSupport(event);
    }
}

bool RichDocumentHandler::evaluateListSupport(QKeyEvent *event)
{
    bool handled = false;
    if (textCursor().currentList()) {
        // handled is False if the key press event was not handled or not completely
        // handled by the Helper class.
        handled = m_nestedListHelper.handleBeforeKeyPressEvent(event, textCursor());
    }

    // If a line was merged with previous (next) one, with different heading level,
    // the style should also be adjusted accordingly (i.e. merged)
    if ((event->key() == Qt::Key_Backspace && textCursor().atBlockStart()
         && (textCursor().blockFormat().headingLevel() != textCursor().block().previous().blockFormat().headingLevel()))
        || (event->key() == Qt::Key_Delete && textCursor().atBlockEnd()
            && (textCursor().blockFormat().headingLevel() != textCursor().block().next().blockFormat().headingLevel()))) {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        if (event->key() == Qt::Key_Delete) {
            cursor.deleteChar();
        } else {
            cursor.deletePreviousChar();
        }

        setHeadingLevel(cursor.blockFormat().headingLevel());
        cursor.endEditBlock();

        // we don't want the native handler to run after this
        return false;
    }

    if (!handled) {
        const bool isControlClicked = event->modifiers() & Qt::ControlModifier;
        const bool isShiftClicked = event->modifiers() & Qt::ShiftModifier;
        if (handleShortcut(event)) {
            event->accept();
            return false;
        } else if (event->key() == Qt::Key_Up && isControlClicked && isShiftClicked) {
            moveLineUpDown(true);
            event->accept();
        } else if (event->key() == Qt::Key_Down && isControlClicked && isShiftClicked) {
            moveLineUpDown(false);
            event->accept();
        } else if (event->key() == Qt::Key_Up && isControlClicked) {
            moveCursorBeginUpDown(true);
            event->accept();
        } else if (event->key() == Qt::Key_Down && isControlClicked) {
            moveCursorBeginUpDown(false);
            event->accept();
        }
        return true;
    }
    return true;
}

bool RichDocumentHandler::isCodeBlock(const QTextBlock &block) const
{
    return block.blockFormat().property(QTextFormat::BlockCodeFence).toBool();
}

void RichDocumentHandler::slotKeyPressed(int key)
{
    // Fetch the cursor once to avoid redundant calls
    auto cursor = textCursor();

    if (key == Qt::Key_Space) {
        // if it's a code block, we don't want to keep everything as is
        if (isCodeBlock(cursor.block())) {
            return;
        }

        const auto fullBlockText = cursor.block().text();

        // Automatic block transformation to header
        if (fullBlockText.startsWith(u'#')) {
            int i = 0;
            while (fullBlockText.length() > i && i < 6 && fullBlockText[i] == u'#') {
                i++;
            }

            cursor.beginEditBlock();
            setHeadingLevel(i);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, i + 1);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // Automatic block transformation to list
        if (fullBlockText.startsWith(u"* ") || fullBlockText.startsWith(u"- ")) {
            cursor.beginEditBlock();
            setListStyle(1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // Automatic block transformation for ordered lists
        static const QRegularExpression orderedListRegex(u"^(\\d+)[\\.|\\)] "_s);
        const auto match = orderedListRegex.match(fullBlockText);
        if (match.hasMatch()) {
            cursor.beginEditBlock();

            const auto capturedNumber = match.captured(1);

            QTextListFormat listFormat;
            listFormat.setStyle(QTextListFormat::ListDecimal);
            listFormat.setStart(capturedNumber.toInt());
            cursor.createList(listFormat);

            const int prefixLength = capturedNumber.length() + 2; // number length + ". "
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, prefixLength);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // Automatic block transformation for check lists
        static const QRegularExpression taskListRegex(u"^\\[([ xX])\\] "_s);
        const auto taskMatch = taskListRegex.match(fullBlockText);
        if (taskMatch.hasMatch()) {
            cursor.beginEditBlock();
            setCheckable(true);

            if (taskMatch.captured(1).toLower() == u"x") {
                QTextBlockFormat fmt = cursor.blockFormat();
                fmt.setMarker(QTextBlockFormat::MarkerType::Checked);
                cursor.setBlockFormat(fmt);
            }

            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // Automatic block transformation to blockquote
        if (fullBlockText.startsWith(u"> ")) {
            cursor.beginEditBlock();
            QTextBlockFormat bfmt = cursor.blockFormat();

            bfmt.setProperty(QTextFormat::BlockQuoteLevel, 1);
            bfmt.setIndent(1);

            cursor.setBlockFormat(bfmt);

            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // indented code block
        if (fullBlockText.startsWith(u"    ")) {
            if (cursor.block().blockNumber() == 0 || cursor.block().previous().text().isEmpty()) {
                cursor.beginEditBlock();

                cursor.movePosition(QTextCursor::StartOfLine);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
                cursor.removeSelectedText();

                applyCodeBlockFormat(cursor.block());
                cursor.endEditBlock();
            }
        }
    }

    if (key != Qt::Key_Return) {
        // Safe length calculation to prevent negative string sizes
        const int textLen = qMax(0, cursor.positionInBlock() - 1);
        const auto textBeforeCursor = cursor.block().text().left(textLen);

        auto transform = [this, &cursor, &textBeforeCursor](const QString &symbol, const QTextCharFormat &format) {
            const auto firstSymbolsInBlock = textBeforeCursor.indexOf(symbol);
            const auto symbolSize = symbol.length();

            if (symbolSize == 1 && textBeforeCursor.indexOf(symbol + symbol) == firstSymbolsInBlock) {
                // Prefer matching with either **text** or __text__ instead of just **text* or __text_
                return;
            }

            if (firstSymbolsInBlock == -1 || !textBeforeCursor.endsWith(symbol) || (firstSymbolsInBlock + symbolSize + 2 >= cursor.positionInBlock())) {
                return;
            }

            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);

            // delete the last instance of the symbol
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, symbolSize);
            cursor.deleteChar();

            // select the text and apply formatting
            const auto selectionSize = cursor.positionInBlock() - (firstSymbolsInBlock + symbolSize);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, selectionSize);

            cursor.mergeCharFormat(format);

            // delete the first instance of the symbol
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, symbolSize);
            cursor.deleteChar();

            // move back to initial position and reset font format
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, selectionSize);

            QTextCharFormat normalFormat;
            normalFormat.setFontWeight(QFont::Normal);
            normalFormat.setFontUnderline(false);
            normalFormat.setFontStrikeOut(false);
            cursor.mergeCharFormat(normalFormat);

            cursor.endEditBlock();
            Q_EMIT cursorPositionChanged();
        };

        // bold
        QTextCharFormat boldFormat;
        boldFormat.setFontWeight(QFont::Bold);
        transform(u"**"_s, boldFormat);
        transform(u"__"_s, boldFormat);

        // italic
        QTextCharFormat italicFormat;
        italicFormat.setFontItalic(true);
        transform(u"*"_s, italicFormat);

        // underline
        QTextCharFormat underlineFormat;
        underlineFormat.setFontUnderline(true);
        transform(u"_"_s, underlineFormat);

        // strikethrough
        QTextCharFormat strikethroughFormat;
        strikethroughFormat.setFontStrikeOut(true);
        transform(u"~~"_s, strikethroughFormat);

        // links
        // Auto-convert Markdown links [text](url)
        const auto textUpToCursor = cursor.block().text().left(cursor.positionInBlock());
        static const QRegularExpression linkRegex(u"\\[([^\\]]+)\\]\\(([^\\)]+)\\)$"_s);
        QRegularExpressionMatch match = linkRegex.match(textUpToCursor);

        if (match.hasMatch()) {
            const QString linkText = match.captured(1);
            const QString linkUrl = match.captured(2);
            const int matchLength = match.capturedLength();

            cursor.beginEditBlock();

            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, matchLength);
            cursor.removeSelectedText();

            QTextCharFormat linkFormat = cursor.charFormat();
            linkFormat.setAnchor(true);
            linkFormat.setAnchorHref(linkUrl);
            linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            linkFormat.setUnderlineColor(linkColor());
            linkFormat.setForeground(linkColor());

            cursor.insertText(linkText, linkFormat);

            QTextCharFormat resetFormat = cursor.charFormat();
            resetFormat.setAnchor(false);
            resetFormat.setAnchorHref(QString());
            resetFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
            resetFormat.clearForeground();

            cursor.setCharFormat(resetFormat);

            cursor.endEditBlock();
            Q_EMIT cursorPositionChanged();
        }
    }

    if (key == Qt::Key_Return) {
        // insert a code block on detecting a code fence
        const auto fullBlockText = cursor.block().previous().text();
        if (fullBlockText.startsWith(u"```")) {
            cursor.beginEditBlock();

            // delete previous line
            cursor.movePosition(QTextCursor::Up);
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.removeSelectedText();
            cursor.deleteChar();

            const QString language = fullBlockText.mid(3).trimmed();
            cursor.insertBlock({}, {});
            applyCodeBlockFormat(cursor.block(), language);

            cursor.endEditBlock();
            return;
        }

        // copy spaces from previous line
        if (isCodeBlock(cursor.block()) && !cursor.block().previous().text().isEmpty() && isCodeBlock(cursor.block().previous())) {
            const auto previousLineText = cursor.block().previous().text();
            const auto leadingSpacesCount = previousLineText.indexOf(QRegularExpression(u"[^ ]"_s));
            if (leadingSpacesCount > 0) {
                cursor.insertText(QString(leadingSpacesCount, u' '));
                return;
            }
        }

        // Clear heading and sticky hyperlink formatting on the new line
        if (cursor.atBlockEnd()) {
            bool formatChanged = false;

            if (cursor.blockFormat().headingLevel() > 0 || cursor.charFormat().isAnchor()) {
                cursor.joinPreviousEditBlock();

                // Clear heading formatting
                if (cursor.blockFormat().headingLevel() > 0) {
                    setHeadingLevel(0);
                    formatChanged = true;
                }

                // Clear sticky hyperlink formatting
                if (cursor.charFormat().isAnchor()) {
                    QTextCharFormat resetFormat = cursor.charFormat();
                    resetFormat.setAnchor(false);
                    resetFormat.setAnchorHref(QString());
                    resetFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
                    resetFormat.clearForeground();

                    cursor.setBlockCharFormat(resetFormat);

                    QTextCursor selectCursor = cursor;
                    selectCursor.select(QTextCursor::BlockUnderCursor);
                    selectCursor.setCharFormat(resetFormat);

                    cursor.setCharFormat(resetFormat);
                    formatChanged = true;
                }

                cursor.endEditBlock();
            }

            if (formatChanged) {
                Q_EMIT cursorPositionChanged();
                reset();
            }
        }
        // Switch checked checkbox to unchecked on the newly created line.
        if (cursor.blockFormat().marker() == QTextBlockFormat::MarkerType::Checked) {
            QTextCursor markerCursor = cursor;
            const QTextBlock previousBlock = cursor.block().previous();
            if (cursor.atBlockStart() && !cursor.block().text().trimmed().isEmpty() && previousBlock.isValid()
                && previousBlock.blockFormat().marker() == QTextBlockFormat::MarkerType::Checked && previousBlock.text().trimmed().isEmpty()) {
                markerCursor = QTextCursor(previousBlock);
            }

            markerCursor.joinPreviousEditBlock();
            QTextBlockFormat bfmt;
            bfmt.setMarker(QTextBlockFormat::MarkerType::Unchecked);
            markerCursor.mergeBlockFormat(bfmt);
            markerCursor.endEditBlock();
            Q_EMIT cursorPositionChanged();
            reset();
        }

        if (cursor.blockFormat().headingLevel() == 0 && !cursor.currentList() && !isCodeBlock(cursor.block()) && !cursor.currentTable()) {
            cursor.joinPreviousEditBlock();
            QTextBlockFormat bfmt = cursor.blockFormat();
            bfmt.setTopMargin(m_blockMargin);
            bfmt.setBottomMargin(0);
            cursor.setBlockFormat(bfmt);
            cursor.endEditBlock();
        }
    }

    if (key == Qt::Key_BracketRight) {
        auto cursor = textCursor();
        if (!cursor.isNull()) {
            const int positionInBlock = cursor.positionInBlock();
            const QString blockText = cursor.block().text().left(positionInBlock);
            const int startIndex = blockText.lastIndexOf("[["_L1);
            if (startIndex != -1 && blockText.endsWith("]]"_L1)) {
                const int linkLength = blockText.length() - startIndex;
                const QString linkBody = blockText.mid(startIndex + 2, linkLength - 4).trimmed();
                if (!linkBody.isEmpty()) {
                    QString noteName = linkBody;
                    QString alias;
                    const int pipeIndex = linkBody.indexOf(u'|');
                    if (pipeIndex != -1) {
                        noteName = linkBody.left(pipeIndex).trimmed();
                        alias = linkBody.mid(pipeIndex + 1).trimmed();
                    }

                    const QString url = internalLinkUrlForName(noteName);
                    if (!noteName.isEmpty() && !url.isEmpty()) {
                        const QString linkText = alias.isEmpty() ? noteName : alias;
                        const int startPos = cursor.block().position() + startIndex;
                        const int endPos = cursor.block().position() + blockText.length();
                        QTextDocument defaultTextDocument;
                        const QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();
                        QTextCharFormat trailingFormat = cursor.charFormat();
                        trailingFormat.setAnchor(false);
                        trailingFormat.setAnchorHref(QString());
                        trailingFormat.setUnderlineStyle(defaultCharFormat.underlineStyle());
                        trailingFormat.setUnderlineColor(defaultCharFormat.underlineColor());
                        trailingFormat.setForeground(normalTextColor());

                        cursor.beginEditBlock();
                        cursor.setPosition(startPos);
                        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
                        cursor.removeSelectedText();
                        cursor.insertText(linkText);

                        cursor.setPosition(startPos);
                        cursor.setPosition(startPos + linkText.length(), QTextCursor::KeepAnchor);

                        QTextCharFormat format = cursor.charFormat();
                        format.setAnchor(true);
                        format.setAnchorHref(url);
                        format.setUnderlineStyle(QTextCharFormat::NoUnderline);
                        format.setForeground(linkColor());
                        cursor.mergeCharFormat(format);

                        cursor.clearSelection();
                        cursor.setPosition(startPos + linkText.length());
                        cursor.insertText(QString(kLinkBoundaryChar), trailingFormat);
                        cursor.endEditBlock();

                        moveCursor(cursor.position());
                        Q_EMIT cursorPositionChanged();
                    }
                }
            }
        }
    }

    // Match the behavior of office suites: newline after header switches to normal text
    if ((key == Qt::Key_Return) && (textCursor().blockFormat().headingLevel() > 0) && (textCursor().atBlockEnd())) {
        // it should be undoable together with actual "return" keypress
        textCursor().joinPreviousEditBlock();
        setHeadingLevel(0);
        textCursor().endEditBlock();
        Q_EMIT cursorPositionChanged();
    }
}

bool RichDocumentHandler::processKeyEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up && e->modifiers() != Qt::ShiftModifier && textCursor().block().position() == 0
        && textCursor().block().layout()->lineForTextPosition(textCursor().position()).lineNumber() == 0) {
        textCursor().clearSelection();
        Q_EMIT focusUp();
        return true;
    }

    // Reset formatting properly when deleting lines
    if (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) {
        QTextCursor cursor = textCursor();
        if (cursor.hasSelection()) {
            QTextCursor cursorAtSelectionStart = textCursor();
            cursorAtSelectionStart.setPosition(cursor.selectionStart());

            if (cursor.selectionStart() == cursorAtSelectionStart.block().position()) {
                cursor.beginEditBlock();
                cursor.setBlockCharFormat(QTextCharFormat());
                cursor.setBlockFormat(QTextBlockFormat());
                cursor.endEditBlock();
            }
        } else if (cursor.positionInBlock() == 0 && textCursor().block().text().trimmed().isEmpty()) {
            // we don't want to merge with previous block if both current and previous blocks are code blocks,
            // otherwise we would lose code block formatting
            if (isCodeBlock(cursor.block()) && isCodeBlock(cursor.block().previous())) {
                return true;
            }

            cursor.beginEditBlock();
            cursor.setBlockCharFormat(QTextCharFormat());
            cursor.setBlockFormat(QTextBlockFormat());
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.removeSelectedText();
            cursor.deletePreviousChar();
            cursor.endEditBlock();
            return false;
        }
    }

    // do not handle any other key events above this
    auto cursor = textCursor();
    if (isCodeBlock(cursor.block())) {
        if (e->key() == Qt::Key_Return) {
            if (e->modifiers().testFlag(Qt::ShiftModifier) || !cursor.block().text().isEmpty() || isCodeBlock(cursor.block().next())) {
                cursor.insertBlock({}, {});
                applyCodeBlockFormat(cursor.block());
            } else if (cursor.block().text().isEmpty()) {
                cursor.setBlockCharFormat({});
                cursor.setBlockFormat({});
                applyParagraphFormat(cursor.block());
            }

            return false;
        }

        if (e->key() == Qt::Key_Tab) {
            cursor.insertText(u"    "_s);
            return false;
        } else if (e->key() == Qt::Key_Backtab) {
            cursor.movePosition(QTextCursor::StartOfLine);
            if (cursor.block().text().startsWith(u"    "_s)) {
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
                cursor.removeSelectedText();
            }

            return false;
        }

        return evaluateReturnKeySupport(e);
    }

    if (cursor.currentList()) {
        if (e->key() == Qt::Key_Return) {
            if (cursor.currentList()->count() == cursor.currentList()->itemNumber(cursor.block()) + 1) {
                if (cursor.block().text().isEmpty()) {
                    indentListLess();
                    return false;
                }
            }
        }
    }

    if (textCursor().currentTable() && (e->key() == Qt::Key_Backtab || e->key() == Qt::Key_Tab)) {
        // navigate to previous or next table cell
        auto cursor = textCursor();
        auto ok = cursor.movePosition(e->key() == Qt::Key_Tab ? QTextCursor::NextCell : QTextCursor::PreviousCell);
        if (!ok) {
            ok = cursor.movePosition(e->key() == Qt::Key_Tab ? QTextCursor::NextBlock : QTextCursor::PreviousBlock);
            moveCursor(cursor.position());
        } else {
            cursor.movePosition(QTextCursor::StartOfLine);
            const int start = cursor.position();
            cursor.movePosition(QTextCursor::EndOfLine);
            const int end = cursor.position();
            selectCursor(start, end);
        }
        return false;
    }

    if (e->key() == Qt::Key_Tab && canIndentList()) {
        indentListMore();
        return false;
    }
    if (e->key() == Qt::Key_Backtab && canDedentList()) {
        indentListLess();
        return false;
    }

    if (e->key() == Qt::Key_Space && e->modifiers() & Qt::ControlModifier && checkable()) {
        auto c = textCursor();
        QTextBlockFormat fmt;
        fmt.setMarker(c.blockFormat().marker() == QTextBlockFormat::MarkerType::Checked ? QTextBlockFormat::MarkerType::Unchecked
                                                                                        : QTextBlockFormat::MarkerType::Checked);
        c.beginEditBlock();
        c.mergeBlockFormat(fmt);
        c.endEditBlock();
        return false;
    }

    return evaluateReturnKeySupport(e);
}

bool RichDocumentHandler::handleShortcut(QKeyEvent *event)
{
    const QKeySequence key = event->modifiers() | (Qt::Key)event->key();

    if (KStandardShortcut::copy().contains(key)) {
        copy();
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        pasteFromClipboard();
        return true;
    } else if (KStandardShortcut::cut().contains(key)) {
        cut();
        return true;
    } else if (KStandardShortcut::undo().contains(key)) {
        undo();
        return true;
    } else if (KStandardShortcut::redo().contains(key)) {
        redo();
        return true;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        deleteWordBack();
        return true;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        deleteWordForward();
        return true;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::PreviousWord);
        moveCursor(cursor.position());
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::NextWord);
        moveCursor(cursor.position());
        return true;
    } else if (KStandardShortcut::begin().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        moveCursor(cursor.position());
        return true;
    } else if (KStandardShortcut::end().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        moveCursor(cursor.position());
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        moveCursor(cursor.position());
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::EndOfLine);
        moveCursor(cursor.position());
        return true;

    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData(QClipboard::Selection);
        if (mimeData) {
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            // Prefer rich HTML if available
            if (mimeData->hasHtml()) {
                cursor.insertHtml(mimeData->html());
            } else if (mimeData->hasFormat(QStringLiteral("text/markdown"))) {
                const QByteArray md = mimeData->data(QStringLiteral("text/markdown"));
                cursor.insertText(QString::fromUtf8(md));
            } else if (mimeData->hasText()) {
                cursor.insertText(mimeData->text());
            }

            cursor.endEditBlock();
        }
        return true;
    }

    return false;
}

void RichDocumentHandler::moveCursorBeginUpDown(bool moveUp)
{
    QTextCursor cursor = textCursor();
    QTextCursor move = cursor;
    move.beginEditBlock();
    cursor.clearSelection();
    move.movePosition(QTextCursor::StartOfBlock);
    move.movePosition(moveUp ? QTextCursor::PreviousBlock : QTextCursor::NextBlock);
    move.endEditBlock();
    setCursorPosition(move.position());
}

void RichDocumentHandler::moveLineUpDown(bool moveUp)
{
    QTextCursor cursor = textCursor();
    QTextCursor move = cursor;
    move.beginEditBlock();

    const bool hasSelection = cursor.hasSelection();

    if (hasSelection) {
        move.setPosition(cursor.selectionStart());
        move.movePosition(QTextCursor::StartOfBlock);
        move.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
        move.movePosition(move.atBlockStart() ? QTextCursor::Left : QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    } else {
        move.movePosition(QTextCursor::StartOfBlock);
        move.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }
    const QString text = move.selectedText();

    move.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    move.removeSelectedText();

    if (moveUp) {
        move.movePosition(QTextCursor::PreviousBlock);
        move.insertBlock();
        move.movePosition(QTextCursor::Left);
    } else {
        move.movePosition(QTextCursor::EndOfBlock);
        if (move.atBlockStart()) { // empty block
            move.movePosition(QTextCursor::NextBlock);
            move.insertBlock();
            move.movePosition(QTextCursor::Left);
        } else {
            move.insertBlock();
        }
    }

    int start = move.position();
    move.clearSelection();
    move.insertText(text);
    int end = move.position();

    if (hasSelection) {
        move.setPosition(end);
        move.setPosition(start, QTextCursor::KeepAnchor);
    } else {
        move.setPosition(start);
    }
    move.endEditBlock();

    setCursorPosition(move.position());
}

void RichDocumentHandler::updateNoteLink(const QString &noteName, const QString &alias)
{
    const QString url = internalLinkUrlForName(noteName);
    const QString linkText = alias.isEmpty() ? noteName : alias;

    updateLink(url, linkText);
}

QString RichDocumentHandler::currentNoteLinkName() const
{
    return internalLinkNameFromUrl(QUrl(currentLinkUrl()));
}

QString RichDocumentHandler::currentNoteLinkAlias() const
{
    return currentLinkText();
}

int RichDocumentHandler::blockMargin() const
{
    return m_blockMargin;
}

void RichDocumentHandler::setBlockMargin(int margin)
{
    if (m_blockMargin == margin) {
        return;
    }
    m_blockMargin = margin;
    parseDocument();
    Q_EMIT blockMarginChanged();
}

void RichDocumentHandler::applyCodeBlockFormat(const QTextBlock &block, const QString &language)
{
    QTextBlockFormat blockFormat{};
    blockFormat.setProperty(QTextFormat::BlockCodeFence, true);

    if (!language.isEmpty()) {
        blockFormat.setProperty(QTextFormat::BlockCodeLanguage, language);
    }

    QTextCharFormat charFormat{};
    charFormat.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    charFormat.setFontFixedPitch(true);
    charFormat.setFontStyleHint(QFont::Monospace);
    charFormat.setBackground(codeBlockBackgroundColor());

    if (block.previous().isValid() && !isCodeBlock(block.previous())) {
        blockFormat.setTopMargin(m_blockMargin);
    }

    QTextCursor cursor(block);
    cursor.setBlockFormat(blockFormat);
    cursor.setBlockCharFormat(charFormat);
}

void RichDocumentHandler::applyHeadingFormat(const QTextBlock &block)
{
    QTextBlockFormat fmt = block.blockFormat();

    const qreal expectedTop = (block == textDocument()->begin()) ? m_blockMargin : (m_blockMargin * 4);
    const qreal expectedBottom = (fmt.headingLevel() == 1) ? (m_blockMargin * 3) : m_blockMargin;

    // Early Bailout
    if (fmt.topMargin() == expectedTop && fmt.bottomMargin() == expectedBottom) {
        return;
    }

    fmt.setTopMargin(expectedTop);
    fmt.setBottomMargin(expectedBottom);

    QTextCursor blockCursor(block);
    blockCursor.setBlockFormat(fmt);
}

void RichDocumentHandler::applyParagraphFormat(const QTextBlock &block)
{
    QTextBlockFormat fmt = block.blockFormat();

    // Do not dirty the block if it's already correct
    if (fmt.topMargin() == m_blockMargin && fmt.bottomMargin() == 2) {
        return;
    }

    fmt.setTopMargin(m_blockMargin);
    fmt.setBottomMargin(2);

    QTextCursor blockCursor(block);
    blockCursor.setBlockFormat(fmt);
}

void RichDocumentHandler::parseDocument()
{
    auto doc = textDocument();
    if (!doc) {
        return;
    }

    for (auto block = doc->begin(); block != doc->end(); block = block.next()) {
        const auto blockFmt = block.blockFormat();

        if (isCodeBlock(block)) {
            const QString language = blockFmt.property(QTextFormat::BlockCodeLanguage).toString();
            applyCodeBlockFormat(block, language);
        } else if (blockFmt.headingLevel() > 0) {
            applyHeadingFormat(block);
        } else if (block.textList()) {
            QTextBlockFormat fmt = block.blockFormat();
            const qreal expectedTop = (block.textList()->itemNumber(block) == 0) ? m_blockMargin : 0;

            if (fmt.topMargin() != expectedTop || fmt.bottomMargin() != 2) {
                fmt.setTopMargin(expectedTop);
                fmt.setBottomMargin(2);
                QTextCursor blockCursor(block);
                blockCursor.setBlockFormat(fmt);
            }
        } else if (!blockFmt.isTableCellFormat()) {
            applyParagraphFormat(block);
        }
    }
}

#include "moc_rich_documenthandler.cpp"
