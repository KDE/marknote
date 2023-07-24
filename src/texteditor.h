// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QObject>
#include <QTextCursor>
class QQuickTextDocument;

class TextEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)

public:
    explicit TextEditor(QObject *parent = nullptr);

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);
    Q_SIGNAL void documentChanged();

    Q_INVOKABLE void makeSelectionItalic();

    Q_SLOT void onCursorPositionChanged(int position);

private:
    QQuickTextDocument *m_document;
    QTextCursor m_cursor;
};

#endif // TEXTEDITOR_H
