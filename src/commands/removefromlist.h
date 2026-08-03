// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMOVEFROMLIST_H
#define REMOVEFROMLIST_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class RemoveFromListCommand : public QUndoCommand
{
public:
    RemoveFromListCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent = nullptr);
    ~RemoveFromListCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    TreeItem *m_listItem;
    TreeItem *m_list;
    TreeItem *m_parentBlock;
    MDTreeModel *m_model;

    int m_listRow = -1;
    int m_listItemRow = -1;
    int m_itemsCount = 0;
    int m_nextSiblingsCount = 0;
    int m_cursorPosition = -1;

    TreeItem *m_newList = nullptr;
    bool m_ownsNewList = false;
    bool m_newListCreated = false;

    bool m_ownsListItem = false;
    bool m_ownsList = false;
};

#endif // REMOVEFROMLIST_H
