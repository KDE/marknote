// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "raw_documenthandler.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KStandardShortcut>

#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QCryptographicHash>
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
#include <QTextCursor>
#include <QTextDocument>

using namespace Qt::StringLiterals;

constexpr int textMargin = 20;

RawDocumentHandler::RawDocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_textArea(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
    , m_lastFontFamily(fontFamily())
    , m_lastFontSize(fontSize())
    , m_lastTextColor(textColor())
{
}

QQuickItem *RawDocumentHandler::textArea() const
{
    return m_textArea;
}

void RawDocumentHandler::setTextArea(QQuickItem *textArea)
{
    if (textArea == m_textArea)
        return;

    m_textArea = textArea;

    if (m_textArea)
        m_textArea->installEventFilter(this);

    Q_EMIT textAreaChanged();
}

QQuickTextDocument *RawDocumentHandler::document() const
{
    return m_document;
}

void RawDocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document == m_document)
        return;

    if (m_document)
        m_document->textDocument()->disconnect(this);
    m_document = document;
    if (m_document)
        connect(m_document->textDocument(), &QTextDocument::modificationChanged, this, &RawDocumentHandler::modifiedChanged);

    Q_EMIT documentChanged();
}

int RawDocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void RawDocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition)
        return;

    m_cursorPosition = position;
    reset();

    Q_EMIT cursorPositionChanged();
}

int RawDocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void RawDocumentHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart)
        return;

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int RawDocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void RawDocumentHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd)
        return;

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QString RawDocumentHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QString();
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

void RawDocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamilies({family});
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    Q_EMIT fontFamilyChanged();
}

QColor RawDocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

void RawDocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    Q_EMIT textColorChanged();
}

int RawDocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void RawDocumentHandler::setFontSize(int size)
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
    cursor.mergeCharFormat(format);
    Q_EMIT fontSizeChanged();
}

QString RawDocumentHandler::fileName() const
{
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
    const QString fileName = QFileInfo(filePath).fileName();
    if (fileName.isEmpty())
        return QStringLiteral("untitled.txt");
    return fileName;
}

QString RawDocumentHandler::fileType() const
{
    return QFileInfo(fileName()).suffix();
}

QUrl RawDocumentHandler::fileUrl() const
{
    return m_fileUrl;
}

void RawDocumentHandler::load(const QUrl &fileUrl)
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

    if (QTextDocument *doc = textDocument()) {
        doc->setUndoRedoEnabled(false);

        const QString content = QString::fromUtf8(file.readAll());

        Q_EMIT loaded(content, Qt::PlainText);

        doc->setModified(false);
        doc->clearUndoRedoStacks();
        doc->setUndoRedoEnabled(true);
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    moveCursor(cursor.position());
    reset();
}

void RawDocumentHandler::saveAs(const QUrl &fileUrl)
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return;

    QFile file(fileUrl.toLocalFile());
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        Q_EMIT error(tr("Cannot save: ") + file.errorString() + u' ' + fileUrl.toLocalFile());
        return;
    }

    const QString content = doc->toPlainText();

    file.write(content.toUtf8());
    file.close();

    if (fileUrl == m_fileUrl)
        return;
    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();
}

void RawDocumentHandler::reset()
{
    if (fontFamily() != m_lastFontFamily) {
        Q_EMIT fontFamilyChanged();
    }
    if (fontSize() != m_lastFontSize) {
        Q_EMIT fontSizeChanged();
    }
    if (textColor() != m_lastTextColor) {
        Q_EMIT textColorChanged();
    }

    m_lastFontFamily = fontFamily();
    m_lastFontSize = fontSize();
    m_lastTextColor = textColor();
}

QTextCursor RawDocumentHandler::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return {};

    QTextCursor cursor(doc);
    int lastValidPos = qMax(0, doc->characterCount() - 1);
    int safePos = qBound(0, m_cursorPosition, lastValidPos);
    cursor.setPosition(safePos);

    if (m_selectionStart != m_selectionEnd) {
        int safeStart = qBound(0, m_selectionStart, lastValidPos);
        int safeEnd = qBound(0, m_selectionEnd, lastValidPos);

        cursor.setPosition(safeStart);
        cursor.setPosition(safeEnd, QTextCursor::KeepAnchor);
    }
    return cursor;
}

QTextDocument *RawDocumentHandler::textDocument() const
{
    if (!m_document)
        return nullptr;
    return m_document->textDocument();
}

bool RawDocumentHandler::modified() const
{
    return m_document && m_document->textDocument()->isModified();
}

void RawDocumentHandler::setModified(bool m)
{
    if (m_document)
        m_document->textDocument()->setModified(m);
}

QString RawDocumentHandler::anchorAt(const QPointF &p) const
{
    return m_document->textDocument()->documentLayout()->anchorAt(p);
}

static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition(op, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void RawDocumentHandler::deleteWordBack()
{
    deleteWord(textCursor(), QTextCursor::PreviousWord);
}

void RawDocumentHandler::deleteWordForward()
{
    deleteWord(textCursor(), QTextCursor::WordRight);
}

void RawDocumentHandler::clearUndoRedoStacks()
{
    if (QTextDocument *doc = textDocument()) {
        doc->clearUndoRedoStacks();
    }
}

#include "moc_raw_documenthandler.cpp"
