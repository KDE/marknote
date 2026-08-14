// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOVEBLOCKCOMMAND_H
#define MOVEBLOCKCOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class MoveBlockCommand : public QUndoCommand
{
public:
    MoveBlockCommand(TreeItem *sourceBlock, TreeItem *targetParent, int targetIndex, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~MoveBlockCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_sourceBlock;
    TreeItem *m_oldParent;
    int m_oldIndex;

    TreeItem *m_targetParent;
    int m_targetIndex;

    MDTreeModel *m_model;

    TreeItem *m_removedEmptySubtree = nullptr;
    int m_removedEmptySubtreeIndex = -1;
    TreeItem *m_removedEmptySubtreeParent = nullptr;

    TreeItem *m_wrappedListItem = nullptr;
    TreeItem *m_wrappedList = nullptr;

    bool m_isValid = true;
};

#endif // MOVEBLOCKCOMMAND_H
