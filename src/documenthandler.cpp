// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "documenthandler.h"

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
#include <QTextCursor>
#include <QTextDocument>

using namespace Qt::StringLiterals;

DocumentHandler::DocumentHandler(QObject *parent)
    : QObject(parent)
{
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

QQuickItem *DocumentHandler::textArea() const
{
    return m_textArea;
}

void DocumentHandler::setTextArea(QQuickItem *textArea)
{
    if (textArea == m_textArea)
        return;

    m_textArea = textArea;

    if (m_textArea) {
        m_textArea->setAcceptHoverEvents(true);
        m_textArea->installEventFilter(this);
    }
    Q_EMIT textAreaChanged();
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

QTextCursor DocumentHandler::textCursor() const
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

QTextDocument *DocumentHandler::textDocument() const
{
    if (!m_document)
        return nullptr;
    return m_document->textDocument();
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

QString DocumentHandler::anchorAt(const QPointF &p) const
{
    return m_document->textDocument()->documentLayout()->anchorAt(p);
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

void DocumentHandler::clearUndoRedoStacks()
{
    if (QTextDocument *doc = textDocument()) {
        doc->clearUndoRedoStacks();
    }
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
}

int DocumentHandler::searchMatchCount() const
{
    return m_searchMatches.size();
}

int DocumentHandler::searchCurrentMatch() const
{
    return m_searchCurrentMatch;
}

int DocumentHandler::findText(const QString &searchTerm)
{
    if (!m_document || searchTerm.isEmpty()) {
        clearSearch();
        return 0;
    }

    m_searchTerm = searchTerm;
    m_searchMatches.clear();
    m_searchCurrentMatch = -1;

    QTextDocument *doc = textDocument();
    if (!doc) {
        return 0;
    }

    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    while (true) {
        cursor = doc->find(searchTerm, cursor);
        if (cursor.isNull()) {
            break;
        }
        m_searchMatches.append(cursor);
    }

    if (!m_searchMatches.isEmpty()) {
        m_searchCurrentMatch = 0;
        QTextCursor firstMatch = m_searchMatches.at(0);
        setCursorPosition(firstMatch.position());
        selectCursor(firstMatch.selectionStart(), firstMatch.selectionEnd());
    }

    Q_EMIT searchMatchCountChanged();
    Q_EMIT searchCurrentMatchChanged();

    return m_searchMatches.size();
}

void DocumentHandler::findNext()
{
    if (m_searchMatches.isEmpty()) {
        return;
    }

    m_searchCurrentMatch = (m_searchCurrentMatch + 1) % m_searchMatches.size();
    QTextCursor match = m_searchMatches.at(m_searchCurrentMatch);
    setCursorPosition(match.position());
    selectCursor(match.selectionStart(), match.selectionEnd());

    Q_EMIT searchCurrentMatchChanged();
}

void DocumentHandler::findPrevious()
{
    if (m_searchMatches.isEmpty()) {
        return;
    }

    m_searchCurrentMatch = (m_searchCurrentMatch - 1 + m_searchMatches.size()) % m_searchMatches.size();
    QTextCursor match = m_searchMatches.at(m_searchCurrentMatch);
    setCursorPosition(match.position());
    selectCursor(match.selectionStart(), match.selectionEnd());

    Q_EMIT searchCurrentMatchChanged();
}

void DocumentHandler::clearSearch()
{
    m_searchTerm.clear();
    m_searchMatches.clear();
    m_searchCurrentMatch = -1;

    Q_EMIT searchMatchCountChanged();
    Q_EMIT searchCurrentMatchChanged();
}

void DocumentHandler::slotMouseMovedWithControl(QPointF position)
{
    // change cursor to a pointer when hovering over a link
    if (m_document && m_textArea) {
        const auto link = m_document->textDocument()->documentLayout()->anchorAt(position);

        if (!link.isEmpty()) {
            m_textArea->setCursor(Qt::PointingHandCursor);
        } else {
            m_textArea->setCursor(Qt::IBeamCursor);
        }
    }
}

void DocumentHandler::slotMouseMovedWithControlReleased()
{
    if (m_textArea) {
        m_textArea->setCursor(Qt::IBeamCursor);
    }
}
