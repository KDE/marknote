// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INSERTCOLUMNINTABLECOMMAND_H
#define INSERTCOLUMNINTABLECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class InsertColumnInTableCommand : public QUndoCommand
{
public:
    InsertColumnInTableCommand(TreeItem *block, int index, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~InsertColumnInTableCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    int m_col;
    int m_insertIndex;
};

#endif // INSERTCOLUMNINTABLECOMMAND_H
