// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEINDENTLISTITEM_H
#define DEINDENTLISTITEM_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class DeIndentListItemCommand : public QUndoCommand
{
public:
    DeIndentListItemCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent = nullptr);
    ~DeIndentListItemCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_listItem;
    TreeItem *m_curList;
    TreeItem *m_targetList;
    int m_row;

    MDTreeModel *m_model;

    // State for undo/redo
    int m_listItemRow = -1;
    int m_parentListItemRow = -1;
    int m_nextSiblingCount = 0;
    TreeItem *m_curListParent = nullptr;
    int m_curListRow = -1;
    bool m_ownsCurList = false;
    TreeItem *m_listItemSubList = nullptr;
    bool m_subListCreated = false;
    bool m_ownsSubList = false;
    int m_cursorPosition = -1;

    static bool isListOrdered(TreeItem *listItem);
};

#endif // DEINDENTLISTITEM_H