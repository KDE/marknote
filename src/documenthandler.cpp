// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2015-2024 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#include "documenthandler.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KStandardShortcut>

#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlFile>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextList>
#include <QTextTable>

using namespace Qt::StringLiterals;

constexpr int textMargin = 20;

DocumentHandler::DocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_textArea(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
{
}

QQuickItem *DocumentHandler::textArea() const
{
    return m_textArea;
}

void DocumentHandler::setTextArea(QQuickItem *textArea)
{
    if (textArea == m_textArea)
        return;

    m_textArea = textArea;

    if (m_textArea)
        m_textArea->installEventFilter(this);

    Q_EMIT textAreaChanged();
}

bool DocumentHandler::eventFilter(QObject *object, QEvent *event)
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

QQuickTextDocument *DocumentHandler::document() const
{
    return m_document;
}

void DocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document == m_document)
        return;

    if (m_document)
        m_document->textDocument()->disconnect(this);
    m_document = document;
    if (m_document)
        connect(m_document->textDocument(), &QTextDocument::modificationChanged, this, &DocumentHandler::modifiedChanged);

    Q_EMIT documentChanged();
}

int DocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void DocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition)
        return;

    m_cursorPosition = position;
    reset();

    Q_EMIT cursorPositionChanged();
}

int DocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void DocumentHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart)
        return;

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int DocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void DocumentHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd)
        return;

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QString DocumentHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QString();
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

void DocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamilies({family});
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontFamilyChanged();
}

QColor DocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

void DocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    Q_EMIT textColorChanged();
}

Qt::Alignment DocumentHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return Qt::AlignLeft;
    return textCursor().blockFormat().alignment();
}

void DocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    Q_EMIT alignmentChanged();
}

bool DocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void DocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT boldChanged();
}

bool DocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

void DocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT italicChanged();
}

bool DocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

void DocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

bool DocumentHandler::strikethrough() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontStrikeOut();
}

void DocumentHandler::setStrikethrough(bool strikethrough)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikethrough);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

int DocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void DocumentHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
        return;

    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontSizeChanged();
}

QString DocumentHandler::fileName() const
{
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
    const QString fileName = QFileInfo(filePath).fileName();
    if (fileName.isEmpty())
        return QStringLiteral("untitled.txt");
    return fileName;
}

QString DocumentHandler::fileType() const
{
    return QFileInfo(fileName()).suffix();
}

QUrl DocumentHandler::fileUrl() const
{
    return m_fileUrl;
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

void DocumentHandler::load(const QUrl &fileUrl)
{
    if (fileUrl == m_fileUrl) {
        return;
    }

    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();

    if (!QFile::exists(fileUrl.toLocalFile())) {
        return;
    }

    QFile file(fileUrl.toLocalFile());
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    QString content;
    m_frontMatter = QString{};

    if (data.startsWith("---") || data.startsWith("***") || data.startsWith("+++")) {
        // we have a front matter
        QTextStream stream(&data);
        QString firstLine;
        int line = 0;
        QString lineContent;
        bool foundEnd = false;

        while (stream.readLineInto(&lineContent)) {
            if (line == 0) {
                firstLine = lineContent;
                m_frontMatter += lineContent + u'\n';
                line++;
                continue;
            }

            line++;
            if (!foundEnd) {
                m_frontMatter += lineContent + u'\n';
                if (lineContent == firstLine) {
                    foundEnd = true;
                }
            } else {
                content += lineContent + u'\n';
            }
        }
    } else {
        content = QString::fromUtf8(data);
    }

    if (QTextDocument *doc = textDocument()) {
        doc->setBaseUrl(QUrl(fileUrl).adjusted(QUrl::RemoveFilename));
        Q_EMIT loaded(content, Qt::MarkdownText);
        doc->setModified(false);
    }

    QSet<int> cursorPositionsToSkip;
    QTextBlock currentBlock = textDocument()->begin();
    QTextBlock::iterator it;
    while (currentBlock.isValid()) {
        for (it = currentBlock.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                const int pos = fragment.position();

                QTextCursor cursor(textDocument());
                cursor.setPosition(pos);
                if (!cursor.currentList() || cursor.currentList()->item(0) == currentBlock) {
                    QTextBlockFormat format;
                    format.setTopMargin(textMargin);
                    cursor.mergeBlockFormat(format);
                }

                QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
                if (imageFormat.isValid()) {
                    if (!cursorPositionsToSkip.contains(pos)) {
                        QTextCursor cursor(textDocument());
                        cursor.setPosition(pos);
                        cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
                        cursor.removeSelectedText();

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
                        cursor.insertHtml(u"<img style=\"max-width: 100%\" src=\""_s + imageFormat.name() + u"\"\\>"_s);
#else
                        cursor.insertHtml(u"<img width=\"500\" src=\""_s + imageFormat.name() + u"\"\\>"_s);
#endif
                        // The textfragment iterator is now invalid, restart from the beginning
                        // Take care not to replace the same fragment again, or we would be in
                        // an infinite loop.
                        cursorPositionsToSkip.insert(pos);
                        // it = currentBlock.begin();
                    }
                }
            }
        }

        currentBlock = currentBlock.next();
    }

    fixupTable(textDocument()->rootFrame());

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    moveCursor(cursor.position());

    reset();
}

void DocumentHandler::saveAs(const QUrl &fileUrl)
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return;

    QFile file(fileUrl.toLocalFile());

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        Q_EMIT error(tr("Cannot save: ") + file.errorString() + u' ' + fileUrl.toLocalFile());
        return;
    }
    if (!m_frontMatter.isEmpty()) {
        file.write(m_frontMatter.toUtf8());
    }
    file.write(doc->toMarkdown().toUtf8());
    file.close();

    if (fileUrl == m_fileUrl)
        return;

    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();
}

void DocumentHandler::reset()
{
    Q_EMIT fontFamilyChanged();
    Q_EMIT alignmentChanged();
    Q_EMIT boldChanged();
    Q_EMIT italicChanged();
    Q_EMIT underlineChanged();
    Q_EMIT fontSizeChanged();
    Q_EMIT textColorChanged();
}

QTextCursor DocumentHandler::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return QTextCursor();

    QTextCursor cursor = QTextCursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

QTextDocument *DocumentHandler::textDocument() const
{
    if (!m_document)
        return nullptr;
    return m_document->textDocument();
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
}

bool DocumentHandler::modified() const
{
    return m_document && m_document->textDocument()->isModified();
}

void DocumentHandler::setModified(bool m)
{
    if (m_document)
        m_document->textDocument()->setModified(m);
}

bool DocumentHandler::canIndentList() const
{
    return m_nestedListHelper.canIndent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

bool DocumentHandler::canDedentList() const
{
    return m_nestedListHelper.canDedent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

int DocumentHandler::currentListStyle() const
{
    if (!textCursor().currentList()) {
        return 0;
    }

    return -textCursor().currentList()->format().style();
}

int DocumentHandler::currentHeadingLevel() const
{
    return textCursor().blockFormat().headingLevel();
}

void DocumentHandler::indentListMore()
{
    m_nestedListHelper.handleOnIndentMore(textCursor());
}

void DocumentHandler::indentListLess()
{
    m_nestedListHelper.handleOnIndentLess(textCursor());
}

void DocumentHandler::setListStyle(int styleIndex)
{
    m_nestedListHelper.handleOnBulletType(-styleIndex, textCursor());
}

void DocumentHandler::setHeadingLevel(int level)
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

QString DocumentHandler::currentLinkUrl() const
{
    return textCursor().charFormat().anchorHref();
}

QString DocumentHandler::currentLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

void DocumentHandler::selectLinkText(QTextCursor *cursor) const
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

void DocumentHandler::updateLink(const QString &linkUrl, const QString &linkText)
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

void DocumentHandler::regenerateColorScheme()
{
    mLinkColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color();
    // TODO update existing link
}

QColor DocumentHandler::linkColor()
{
    if (mLinkColor.isValid()) {
        return mLinkColor;
    }
    regenerateColorScheme();
    return mLinkColor;
}

void DocumentHandler::insertImage(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return;
    }

    QImage image;
    if (!image.load(url.path())) {
        return;
    }

    // Ensure we are putting the image in a new line and not in a list has it
    // breaks the Qt rendering
    textCursor().insertHtml(u"<br />"_s);

    while (canDedentList()) {
        m_nestedListHelper.handleOnIndentLess(textCursor());
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    textCursor().insertHtml(u"<img style=\"max-width: 100%\" src=\""_s + url.path() + u"\"\\>"_s);
#else
    textCursor().insertHtml(u"<img width=\"500\" src=\""_s + url.path() + u"\"\\>"_s);
#endif
}

void DocumentHandler::insertTable(int rows, int columns)
{
    QString htmlText;

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

void DocumentHandler::setCheckable(bool add)
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

bool DocumentHandler::checkable() const
{
    return textCursor().blockFormat().marker() == QTextBlockFormat::MarkerType::Unchecked
        || textCursor().blockFormat().marker() == QTextBlockFormat::MarkerType::Checked;
}

bool DocumentHandler::evaluateReturnKeySupport(QKeyEvent *event)
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

bool DocumentHandler::evaluateListSupport(QKeyEvent *event)
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

void DocumentHandler::slotKeyPressed(int key)
{
    if (key == Qt::Key_Space) {
        const auto blockText = textCursor().block().text();

        // Automatic block transformation to header
        if (blockText.startsWith(u'#')) {
            int i = 0;
            while (blockText.length() > i && i < 6 && blockText[i] == u'#') {
                i++;
            }

            auto cursor = textCursor();
            cursor.beginEditBlock();
            setHeadingLevel(i);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, i + 1);
            cursor.deleteChar();
            cursor.endEditBlock();
        }

        // Automatic block transformation to list
        if (blockText.startsWith(u"* ") || blockText.startsWith(u"- ")) {
            auto cursor = textCursor();
            cursor.beginEditBlock();
            setListStyle(1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 2);
            cursor.deleteChar();
            cursor.endEditBlock();
        }
    }

    if (key != Qt::Key_Return) {
        auto cursor = textCursor();
        const auto blockText = cursor.block().text().left(cursor.positionInBlock() - 1);

        auto transform = [this, &cursor, &blockText](const QString &symbol, const QTextCharFormat &format) {
            const auto firstSymbolsInBlock = blockText.indexOf(symbol);
            const auto symbolSize = symbol.length();

            if (symbolSize == 1 && blockText.indexOf(symbol + symbol) == firstSymbolsInBlock) {
                // Prefer matching with either **text** or __text__ instead of just **text* or __text_
                return;
            }

            if (firstSymbolsInBlock == -1 || !blockText.endsWith(symbol) || (firstSymbolsInBlock + symbolSize + 2 >= cursor.positionInBlock())) {
                return;
            }

            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);

            // delete the last instance of the symbol
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, symbolSize);
            cursor.deleteChar();

            // select the text and bold it
            const auto selectionSize = cursor.positionInBlock() - (firstSymbolsInBlock + symbolSize);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, selectionSize);

            cursor.mergeCharFormat(format);

            // delete the first instance of the symbol
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, symbolSize);
            cursor.deleteChar();

            // move back to initial position and font weight
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
    if ((key == Qt::Key_Return) && (textCursor().blockFormat().headingLevel() > 0) && (textCursor().atBlockEnd())) {
        // it should be undoable together with actual "return" keypress
        textCursor().joinPreviousEditBlock();
        setHeadingLevel(0);
        textCursor().endEditBlock();
        Q_EMIT cursorPositionChanged();
    }

    if (textCursor().currentList()) {
        if ((key != Qt::Key_Backspace) && (key != Qt::Key_Return)) {
            return;
        }

        auto cursor = textCursor();
        QTextBlock currentBlock = cursor.block();
        if (cursor.currentList()->count() == cursor.currentList()->itemNumber(currentBlock) + 1) {
            if (cursor.currentList()->count() > 1 && cursor.currentList()->itemNumber(currentBlock)) {
                if (currentBlock.previous().text().isEmpty()) {
                    cursor.joinPreviousEditBlock();
                    QTextBlockFormat bfmt;
                    bfmt.setTopMargin(textMargin);
                    bfmt.setBottomMargin(0);
                    textCursor().setBlockFormat(bfmt);
                    textCursor().endEditBlock();
                    return;
                }
            }
        }

        cursor.joinPreviousEditBlock();
        QTextBlockFormat bfmt = textCursor().block().blockFormat();
        bfmt.setTopMargin(cursor.block().previous().textList() == nullptr ? textMargin : 0);
        bfmt.setBottomMargin(0);
        textCursor().setBlockFormat(bfmt);
        textCursor().endEditBlock();
    }
}

bool DocumentHandler::processKeyEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up && e->modifiers() != Qt::ShiftModifier && textCursor().block().position() == 0
        && textCursor().block().layout()->lineForTextPosition(textCursor().position()).lineNumber() == 0) {
        textCursor().clearSelection();
        Q_EMIT focusUp();
    } else {
        QTextTable *table = textCursor().currentTable();
        const bool isTable = (table != nullptr);
        if (isTable && (e->key() == Qt::Key_Backtab || e->key() == Qt::Key_Tab)) {
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
        } else {
            return evaluateReturnKeySupport(e);
        }
    }
    return true;
}

bool DocumentHandler::handleShortcut(QKeyEvent *event)
{
    const QKeySequence key = event->modifiers() | (Qt::Key)event->key();

    if (KStandardShortcut::copy().contains(key)) {
        copy();
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        textCursor().insertText(QGuiApplication::clipboard()->text(QClipboard::Clipboard));
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
        //} else if (searchSupport() && KStandardShortcut::find().contains(key)) {
        //    Q_EMIT findText();
        //    return true;
        //} else if (searchSupport() && KStandardShortcut::replace().contains(key)) {
        //    if (!isReadOnly()) {
        //        Q_EMIT replaceText();
        //    }
        //    return true;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        QString text = QGuiApplication::clipboard()->text(QClipboard::Selection);
        if (!text.isEmpty()) {
            textCursor().insertText(text); // TODO: check if this is html? (MiB)
        }
        return true;
    } else if (event == QKeySequence::DeleteEndOfLine) {
        QTextCursor cursor = textCursor();
        QTextBlock block = cursor.block();
        if (cursor.position() == block.position() + block.length() - 2) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        } else {
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        }
        cursor.removeSelectedText();
        moveCursor(cursor.position());
        return true;
    }

    return false;
}

void DocumentHandler::moveCursorBeginUpDown(bool moveUp)
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

void DocumentHandler::moveLineUpDown(bool moveUp)
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

static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition(op, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void DocumentHandler::deleteWordBack()
{
    deleteWord(textCursor(), QTextCursor::PreviousWord);
}

void DocumentHandler::deleteWordForward()
{
    deleteWord(textCursor(), QTextCursor::WordRight);
}

#include "moc_documenthandler.cpp"
