// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMOVEBLOCKSCOMMAND_H
#define REMOVEBLOCKSCOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QList>
#include <QUndoCommand>

class RemoveBlocksCommand : public QUndoCommand
{
public:
    RemoveBlocksCommand(const QList<TreeItem *> &blocks, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~RemoveBlocksCommand();

    void undo() override;
    void redo() override;

private:
    struct Removal {
        TreeItem *item;
        TreeItem *parent;
        int index;
    };

    MDTreeModel *m_model;
    QList<TreeItem *> m_blocksToRemove;
    QList<Removal> m_removals;
    bool m_firstTime;
};

#endif // REMOVEBLOCKSCOMMAND_H
