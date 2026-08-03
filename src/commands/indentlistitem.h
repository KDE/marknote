// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INDENTLISTITEM_H
#define INDENTLISTITEM_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class IndentListItemCommand : public QUndoCommand
{
public:
    IndentListItemCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent = nullptr);
    ~IndentListItemCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_listItem;
    TreeItem *m_curList;
    TreeItem *m_prevListItem = nullptr;
    TreeItem *m_targetSubList = nullptr;

    MDTreeModel *m_model;

    int m_listItemRow = -1;
    bool m_targetSubListCreated = false;
    bool m_ownsTargetSubList = false;
    int m_cursorPosition = -1;
};

#endif // INDENTLISTITEM_H
