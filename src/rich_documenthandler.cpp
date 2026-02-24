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

#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QCryptographicHash>
#include <QCursor>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPalette>
#include <QQmlFile>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextList>
#include <QTextTable>

using namespace Qt::StringLiterals;

constexpr int textMargin = 20;

RichDocumentHandler::RichDocumentHandler(QObject *parent)
    : DocumentHandler(parent)
{
    m_document = nullptr;
    m_textArea = nullptr;
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
}

bool RichDocumentHandler::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_textArea && event->type() == QEvent::KeyPress) {
        return !processKeyEvent(static_cast<QKeyEvent *>(event));
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
            QDesktopServices::openUrl(QUrl(link));
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
    const auto children = frame->childFrames();
    for (const auto child : children) {
        if (auto table = dynamic_cast<QTextTable *>(child)) {
            QTextTableFormat tableFormat;
            tableFormat.setBorder(1);
            const int numberOfColumns(table->columns());
            QList<QTextLength> constrains;
            constrains.reserve(numberOfColumns);
            const QTextLength::Type type = QTextLength::PercentageLength;
            const qreal length = 100; // 100% of window width

            const QTextLength textlength(type, length / numberOfColumns);
            for (int i = 0; i < numberOfColumns; ++i) {
                constrains.append(textlength);
            }
            tableFormat.setColumnWidthConstraints(constrains);
            tableFormat.setAlignment(Qt::AlignLeft);
            tableFormat.setCellPadding(4);
            tableFormat.setBorder(0.5);
            tableFormat.setBorderCollapse(true);
            tableFormat.setTopMargin(textMargin);
            table->setFormat(tableFormat);
        }
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

    const QString content = QString::fromUtf8(file.readAll());

    // PERFORMANCE WIN: String Builder Pattern
    // Instead of replace() inside a loop (O(N^2)), we build the new string
    // (O(N)).
    QString processedContent;
    processedContent.reserve(content.length() + (content.length() / 5));

    m_imagePathLookup.clear();

    if (QTextDocument *doc = textDocument()) {
        doc->setUndoRedoEnabled(false);
        doc->setBaseUrl(QUrl(fileUrl).adjusted(QUrl::RemoveFilename));

        static const QRegularExpression imgRegex(u"!\\[.*?\\]\\(([^)]+)\\)"_s, QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatchIterator i = imgRegex.globalMatch(content);

        int lastPos = 0; // Track where we are in the original string

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();

            // OPTIMIZATION: Zero-Copy View
            QStringView originalPathView = match.capturedView(1);
            originalPathView = originalPathView.trimmed();

            int quoteIndex = originalPathView.indexOf(u" \"");
            if (quoteIndex == -1)
                quoteIndex = originalPathView.indexOf(u" '");
            if (quoteIndex != -1)
                originalPathView = originalPathView.left(quoteIndex).trimmed();

            if (originalPathView.startsWith(u'<') && originalPathView.endsWith(u'>')) {
                originalPathView = originalPathView.mid(1, originalPathView.length() - 2);
            }

            // Append text before the image
            processedContent.append(QStringView(content).mid(lastPos, match.capturedStart() - lastPos));

            // Process Image
            // FIX: Use doc->baseUrl() directly to avoid scope errors
            QUrl absoluteUrl = doc->baseUrl().resolved(QUrl(originalPathView.toString()));
            QString proxyUrl = processImage(absoluteUrl);

            processedContent.append(u"<br />"_s);

// Append HTML (Inline construction to avoid temp objects)
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
            processedContent.append(u"<img style=\"max-width: 100%\" src=\""_s);
#else
            processedContent.append(u"<img width=\"500\" src=\""_s);
#endif
            processedContent.append(proxyUrl);
            processedContent.append(u"\" />"_s);

            processedContent.append(u"<br />"_s);

            lastPos = match.capturedEnd();
        }

        // Append remaining text
        processedContent.append(QStringView(content).mid(lastPos));

        Q_EMIT loaded(processedContent, Qt::MarkdownText);

        doc->setModified(false);
        doc->clearUndoRedoStacks();
        doc->setUndoRedoEnabled(true);
    }
    fixupTable(textDocument()->rootFrame());
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    moveCursor(cursor.position());
    reset();
}

void RichDocumentHandler::saveAs(const QUrl &fileUrl)
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return;

    QFile file(fileUrl.toLocalFile());

    const QString markdown = doc->toMarkdown();

    // PERFORMANCE WIN: String Builder for Saving
    QString finalOutput;
    finalOutput.reserve(markdown.length());

    static const QRegularExpression linkRegex(u"\\]\\(image://marknote/([a-f0-9]+)[^)]*\\)"_s);
    QRegularExpressionMatchIterator i = linkRegex.globalMatch(markdown);

    int lastPos = 0;

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString hash = match.captured(1);

        // Append text before the link
        finalOutput.append(QStringView(markdown).mid(lastPos, match.capturedStart() - lastPos));

        if (m_imagePathLookup.contains(hash)) {
            QString originalPath = m_imagePathLookup.value(hash);
            QString replacement = u"]("_s + originalPath + u")"_s;
            finalOutput.append(replacement);
        } else {
            // Should not happen, but keep original if hash not found
            finalOutput.append(match.captured());
        }

        lastPos = match.capturedEnd();
    }

    // Append remaining text
    finalOutput.append(QStringView(markdown).mid(lastPos));
    QFile fileCheck(fileUrl.toLocalFile());
    if (fileCheck.exists() && fileCheck.open(QFile::ReadOnly)) {
        const QByteArray existingContent = fileCheck.readAll();
        fileCheck.close();

        if (existingContent == finalOutput.toUtf8()) {
            if (fileUrl != m_fileUrl) {
                m_fileUrl = fileUrl;
                Q_EMIT fileUrlChanged();
            }
            doc->setModified(false);
            return;
        }
    }

    // We only reach here if the content is actually different.
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        Q_EMIT error(tr("Cannot save: ") + file.errorString() + u' ' + fileUrl.toLocalFile());
        return;
    }

    file.write(finalOutput.toUtf8());
    file.close();

    if (fileUrl == m_fileUrl) {
        doc->setModified(false);
        return;
    }

    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();
    doc->setModified(false);
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
    const int boundedLevel = qBound(0, 6, level);
    // Apparently, 5 is maximum for FontSizeAdjustment; otherwise level=1 and
    // level=2 look the same
    const int sizeAdjustment = boundedLevel > 0 ? 5 - boundedLevel : 0;

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blkfmt;
    blkfmt.setHeadingLevel(boundedLevel);
    cursor.mergeBlockFormat(blkfmt);

    QTextCharFormat chrfmt;
    chrfmt.setFontWeight(boundedLevel > 0 ? QFont::Bold : QFont::Normal);
    chrfmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
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
    } else {
        selectCursor.select(QTextCursor::BlockUnderCursor);
    }
    selectCursor.mergeCharFormat(chrfmt);

    cursor.mergeBlockCharFormat(chrfmt);
    cursor.endEditBlock();
    // richTextComposer()->setTextCursor(cursor);
    // richTextComposer()->setFocus();
    // richTextComposer()->activateRichText();
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
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(linkColor());
        format.setForeground(linkColor());
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        QTextDocument defaultTextDocument;
        QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();

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
    mLinkColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color();
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
        QMutexLocker locker(&s_mutex);
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
    tableFormat.setBorder(1);
    const int numberOfColumns(columns);
    QList<QTextLength> constrains;
    constrains.reserve(numberOfColumns);
    const QTextLength::Type type = QTextLength::PercentageLength;
    const int length = 100; // 100% of window width

    const QTextLength textlength(type, length / numberOfColumns);
    for (int i = 0; i < numberOfColumns; ++i) {
        constrains.append(textlength);
    }
    tableFormat.setColumnWidthConstraints(constrains);
    tableFormat.setAlignment(Qt::AlignLeft);
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(4);
    tableFormat.setBorderCollapse(true);
    tableFormat.setBorder(0.5);
    tableFormat.setTopMargin(20);

    Q_ASSERT(cursor.document());
    QTextTable *table = cursor.insertTable(rows, numberOfColumns, tableFormat);

    // fill table with whitespace
    for (int i = 0, rows = table->rows(); i < rows; i++) {
        for (int j = 0, columns = table->columns(); j < columns; j++) {
            auto cell = table->cellAt(i, j);
            Q_ASSERT(cell.isValid());
            cell.firstCursorPosition().insertText(u" "_s);
        }
    }
    return;
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
        handled = true;
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

void RichDocumentHandler::slotKeyPressed(int key)
{
    // Fetch the cursor once to avoid redundant calls
    auto cursor = textCursor();

    if (key == Qt::Key_Space) {
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
    }

    // Match the behavior of office suites: newline after header switches to normal text
    if ((key == Qt::Key_Return) && (cursor.blockFormat().headingLevel() > 0) && (cursor.atBlockEnd())) {
        // it should be undoable together with actual "return" keypress
        cursor.joinPreviousEditBlock();
        setHeadingLevel(0);
        cursor.endEditBlock();
        Q_EMIT cursorPositionChanged();
    }

    if (cursor.currentList()) {
        if ((key != Qt::Key_Backspace) && (key != Qt::Key_Return)) {
            return;
        }

        QTextBlock currentBlock = cursor.block();
        if (cursor.currentList()->count() == cursor.currentList()->itemNumber(currentBlock) + 1) {
            if (cursor.currentList()->count() > 1 && cursor.currentList()->itemNumber(currentBlock)) {
                if (currentBlock.previous().text().isEmpty()) {
                    cursor.joinPreviousEditBlock();
                    QTextBlockFormat bfmt;
                    bfmt.setTopMargin(textMargin);
                    bfmt.setBottomMargin(0);
                    cursor.setBlockFormat(bfmt);
                    cursor.endEditBlock();
                    return;
                }
            }
        }

        cursor.joinPreviousEditBlock();
        QTextBlockFormat bfmt = cursor.block().blockFormat();
        bfmt.setTopMargin(cursor.block().previous().textList() == nullptr ? textMargin : 0);
        bfmt.setBottomMargin(0);
        cursor.setBlockFormat(bfmt);
        cursor.endEditBlock();
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
        const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard);
        if (!mimeData) {
            return true;
        }

        if (const auto urls = mimeData->urls(); urls.size() == 1 && urls.front().scheme() == "https"_L1) {
            updateLink(urls.front().toString(), QString());
            return true;
        }

        // Prefer rich HTML if available
        if (mimeData->hasHtml()) {
            textCursor().insertHtml(mimeData->html());
            return true;
        }

        if (mimeData->hasFormat(QStringLiteral("text/markdown"))) {
            const QByteArray md = mimeData->data(QStringLiteral("text/markdown"));
            textCursor().insertText(QString::fromUtf8(md));
            return true;
        }

        const QString text = mimeData->text();
        if (const QUrl url(text, QUrl::StrictMode); url.isValid() && url.scheme() == "https"_L1) {
            updateLink(url.toString(), QString());
            return true;
        }
        // i return false here to let the QML TextArea's native handler take over it for ctrl+v pasting(fix: causing the text to be pasted twice).
        return false;
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

#include "moc_rich_documenthandler.cpp"
