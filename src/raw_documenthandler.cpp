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

RawDocumentHandler::RawDocumentHandler(QObject *parent)
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

void RawDocumentHandler::pasteFromClipboard()
{
    // const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    // if (!mimeData) {
    //     return;
    // }
    //
    // if (mimeData->hasUrls()) {
    //     bool pastedImage = false;
    //     const QList<QUrl> urls = mimeData->urls();
    //
    //     for (const QUrl &url : urls) {
    //         if (url.isLocalFile()) {
    //             QMimeDatabase db;
    //             const QMimeType mimeType = db.mimeTypeForFile(url.toLocalFile());
    //
    //             if (mimeType.name().startsWith(u"image/"_s)) {
    //                 insertImage(url);
    //                 pastedImage = true;
    //             }
    //         }
    //     }
    //
    //     // If we successfully intercepted and pasted image(s), stop here
    //     // so we don't duplicate them as raw text strings.
    //     if (pastedImage) {
    //         return;
    //     }
    // }
    //
    // QTextCursor cursor = textCursor();
    // cursor.beginEditBlock();
    //
    // if (mimeData->hasHtml()) {
    //     cursor.insertHtml(mimeData->html());
    // } else if (mimeData->hasFormat(QStringLiteral("text/markdown"))) {
    //     const QByteArray md = mimeData->data(QStringLiteral("text/markdown"));
    //     cursor.insertText(QString::fromUtf8(md));
    // } else if (mimeData->hasText()) {
    //     cursor.insertText(mimeData->text());
    // }
    //
    // cursor.endEditBlock();
}

#include "moc_raw_documenthandler.cpp"
