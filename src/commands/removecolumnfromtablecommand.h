// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMOVECOLUMNFROMTABLECOMMAND_H
#define REMOVECOLUMNFROMTABLECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class RemoveColumnFromTableCommand : public QUndoCommand
{
public:
    RemoveColumnFromTableCommand(TreeItem *block, int col, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~RemoveColumnFromTableCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    int m_col;

    QList<QSharedPointer<MD::TableCell>> m_savedColData;
    QList<QString> m_unparsedColData;
};

#endif // REMOVECOLUMNFROMTABLECOMMAND_H
