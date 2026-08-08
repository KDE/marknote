// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMOVEROWFROMTABLECOMMAND_H
#define REMOVEROWFROMTABLECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class RemoveRowFromTableCommand : public QUndoCommand
{
public:
    RemoveRowFromTableCommand(TreeItem *block, int row, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~RemoveRowFromTableCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    int m_row;

    QSharedPointer<MD::TableRow> m_savedRowData;
    QList<QString> m_unparsedRowData;
};

#endif // REMOVEROWFROMTABLECOMMAND_H
