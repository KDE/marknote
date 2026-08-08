// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "mdtreemodel/mdtreemodel.h"
#include <QObject>
#include <QUndoStack>
#include <QtQmlIntegration/qqmlintegration.h>

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager(QObject *parent = nullptr);
    ~CommandManager();

    Q_INVOKABLE void setModel(MDTreeModel *model);

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE bool canUndo() const;
    Q_INVOKABLE bool canRedo() const;

    Q_INVOKABLE void editText(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void editCode(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void
    editTableCellText(TreeItem *block, int row, int column, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void parseBlock(TreeItem *block, const QString &text);

    Q_INVOKABLE void insertRowInTable(TreeItem *block);
    Q_INVOKABLE void insertColInTable(TreeItem *block);
    Q_INVOKABLE void deleteRowInTable(TreeItem *block, int row);
    Q_INVOKABLE void deleteColumnInTable(TreeItem *block, int col);

    Q_INVOKABLE void splitBlock(TreeItem *block, const QString &text, int splitIndex);
    Q_INVOKABLE void splitCode(TreeItem *block, const QString &oldText, const QString &text, int cursorPosition);
    Q_INVOKABLE void insertParagraphBelow(TreeItem *block, const QString &text);
    Q_INVOKABLE void mergeWithPreviousBlock(TreeItem *block, const QString &text);
    Q_INVOKABLE void transformToBlockquote(TreeItem *block, int level, const QString &text);
    Q_INVOKABLE void transformToList(TreeItem *block, bool isOrdered, int startNumber, const QString &text);
    Q_INVOKABLE void transformToChecklist(TreeItem *block, bool isChecked, const QString &text);
    Q_INVOKABLE void splitListItem(TreeItem *block, const QString &text, int splitIndex);
    Q_INVOKABLE bool deIndentListItem(TreeItem *block, int cursorPosition);
    Q_INVOKABLE void indentListItem(TreeItem *block, int cursorPosition);
    Q_INVOKABLE bool convertToParagraph(TreeItem *block, int cursorPosition);
    Q_INVOKABLE bool moveOutsideBlockquote(TreeItem *block, int cursorPosition);

    Q_INVOKABLE void moveToPreviousBlock(TreeItem *block, const QString &currentText, int cursorPosition);
    Q_INVOKABLE void moveToNextBlock(TreeItem *block, const QString &currentText, int cursorPosition);

    Q_INVOKABLE void moveToLeftTableCell(TreeItem *block, int row, int column);
    Q_INVOKABLE void moveToRightTableCell(TreeItem *block, int row, int column);
    Q_INVOKABLE void moveToTopTableCell(TreeItem *block, int row, int column, int cursorPosition);
    Q_INVOKABLE void moveToBottomTableCell(TreeItem *block, int row, int column, int cursorPosition);

    Q_INVOKABLE bool autoTransform(TreeItem *block, const QString &text, int cursorPosition, int index);

    Q_INVOKABLE int getCursorInMdString(const QString &rawString, const QString &mdString, int index);

private:
    TreeItem *getPreviousSibling(TreeItem *block);
    TreeItem *getNextSibling(TreeItem *block);

    bool removeBlockquoteIfAtStart(TreeItem *bqBlock, TreeItem *block, int cursorPosition);
    bool removeFromListIfAtStart(TreeItem *block, int cursorPosition);

    QString getBlockText(TreeItem *block) const;

    QUndoStack m_undoStack;
    MDTreeModel *m_model;
};

#endif // COMMANDMANAGER_H