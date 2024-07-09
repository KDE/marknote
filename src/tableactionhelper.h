/*
  SPDX-FileCopyrightText: 2012-2024 Laurent Montel <montel@kde.org>
  SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#pragma once

#include <QAction>
#include <QObject>
#include <QQuickTextDocument>
#include <QTextDocument>
#include <QtQml>

class TableActionHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QAction *actionInsertRowBelow MEMBER actionInsertRowBelow CONSTANT)
    Q_PROPERTY(QAction *actionInsertRowAbove MEMBER actionInsertRowAbove CONSTANT)

    Q_PROPERTY(QAction *actionInsertColumnBefore MEMBER actionInsertColumnBefore CONSTANT)
    Q_PROPERTY(QAction *actionInsertColumnAfter MEMBER actionInsertColumnAfter CONSTANT)

    Q_PROPERTY(QAction *actionRemoveRow MEMBER actionRemoveRow CONSTANT)
    Q_PROPERTY(QAction *actionRemoveColumn MEMBER actionRemoveColumn CONSTANT)

    Q_PROPERTY(QAction *actionRemoveCellContents MEMBER actionRemoveCellContents CONSTANT)

public:
    explicit TableActionHelper(QObject *parent = nullptr);
    ~TableActionHelper() override;

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    int cursorPosition() const;
    void setCursorPosition(int position);

    int selectionStart() const;
    void setSelectionStart(int position);

    int selectionEnd() const;
    void setSelectionEnd(int position);

Q_SIGNALS:
    void cursorPositionChanged();
    void documentChanged();
    void selectionStartChanged();
    void selectionEndChanged();

private:
    QTextCursor textCursor() const;
    QTextDocument *textDocument() const;

    void _k_slotInsertRowBelow();
    void _k_slotInsertRowAbove();
    void _k_slotInsertColumnBefore();
    void _k_slotInsertColumnAfter();

    void _k_slotRemoveRow();
    void _k_slotRemoveColumn();
    void _k_slotRemoveCellContents();

    void _k_updateActions(bool forceUpdate = false);

    QAction *actionInsertRowBelow = nullptr;
    QAction *actionInsertRowAbove = nullptr;

    QAction *actionInsertColumnBefore = nullptr;
    QAction *actionInsertColumnAfter = nullptr;

    QAction *actionRemoveRow = nullptr;
    QAction *actionRemoveColumn = nullptr;

    QAction *actionRemoveCellContents = nullptr;

    QQuickTextDocument *m_document = nullptr;
    int m_cursorPosition = 0;
    int m_selectionStart = 0;
    int m_selectionEnd = 0;
};
