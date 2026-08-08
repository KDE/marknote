// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INSERTROWINTABLECOMMAND_H
#define INSERTROWINTABLECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class InsertRowInTableCommand : public QUndoCommand
{
public:
    InsertRowInTableCommand(TreeItem *block, int index, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~InsertRowInTableCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    int m_row;
    int m_insertIndex;
};

#endif // INSERTROWINTABLECOMMAND_H
