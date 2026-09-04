// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "mdtreemodel/mdtreemodel.h"
#include <QHash>
#include <QObject>
#include <QUndoStack>
#include <QtQmlIntegration/qqmlintegration.h>

class CommandManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MDTreeModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)

public:
    CommandManager(QObject *parent = nullptr);
    ~CommandManager();

    MDTreeModel *model() const;
    Q_INVOKABLE void setModel(MDTreeModel *model);

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    bool canUndo() const;
    bool canRedo() const;

    Q_INVOKABLE void editText(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void editCode(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void
    editTableCellText(TreeItem *block, int row, int column, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition);
    Q_INVOKABLE void parseBlock(TreeItem *block, const QString &text);

    Q_INVOKABLE void insertRowInTable(TreeItem *block, int index = -1);
    Q_INVOKABLE void insertColInTable(TreeItem *block, int index = -1);
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
    Q_INVOKABLE void changeListType(TreeItem *block, int listType);
    Q_INVOKABLE bool isListItem(TreeItem *block) const;
    Q_INVOKABLE bool canDeIndentListItem(TreeItem *block) const;
    Q_INVOKABLE bool deIndentListItem(TreeItem *block, int cursorPosition);
    Q_INVOKABLE bool canIndentListItem(TreeItem *block) const;
    Q_INVOKABLE void indentListItem(TreeItem *block, int cursorPosition);
    Q_INVOKABLE bool convertToParagraph(TreeItem *block, int cursorPosition);
    Q_INVOKABLE bool moveOutsideBlockquote(TreeItem *block, int cursorPosition);
    Q_INVOKABLE void moveBlock(TreeItem *sourceBlock, TreeItem *targetParent, int targetIndex);
    Q_INVOKABLE bool isValidMove(TreeItem *sourceBlock, TreeItem *targetParent, int targetIndex);

    Q_INVOKABLE void removeBlocks(const QList<TreeItem *> &blocks);
    Q_INVOKABLE QString blocksToMarkdown(const QList<TreeItem *> &blocks) const;

    Q_INVOKABLE void moveToPreviousBlock(TreeItem *block, const QString &currentText, int cursorPosition);
    Q_INVOKABLE void moveToNextBlock(TreeItem *block, const QString &currentText, int cursorPosition);

    Q_INVOKABLE void moveToLeftTableCell(TreeItem *block, int row, int column);
    Q_INVOKABLE void moveToRightTableCell(TreeItem *block, int row, int column);
    Q_INVOKABLE void moveToTopTableCell(TreeItem *block, int row, int column, int cursorPosition);
    Q_INVOKABLE void moveToBottomTableCell(TreeItem *block, int row, int column, int cursorPosition);

    Q_INVOKABLE bool autoTransform(TreeItem *block, const QString &text, int cursorPosition, int index);

    Q_INVOKABLE int getCursorInMdString(const QString &rawString, const QString &mdString, int index);

    Q_INVOKABLE void handleLink(const QString &linkString);

Q_SIGNALS:
    void modelChanged();
    void canUndoChanged();
    void canRedoChanged();
    void internalLinkActivated(const QString &noteName);

private:
    TreeItem *getPreviousSibling(TreeItem *block);
    TreeItem *getNextSibling(TreeItem *block);

    bool removeBlockquoteIfAtStart(TreeItem *bqBlock, TreeItem *block, int cursorPosition);
    bool removeFromListIfAtStart(TreeItem *block, int cursorPosition);

    QString getBlockText(TreeItem *block) const;

    QUndoStack *currentUndoStack() const;
    void pushCommand(QUndoCommand *command);

    QHash<MDTreeModel *, QUndoStack *> m_undoStacks;
    MDTreeModel *m_model;
};

#endif // COMMANDMANAGER_H