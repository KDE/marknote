// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MERGEWITHPREVIOUSBLOCK_H
#define MERGEWITHPREVIOUSBLOCK_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class MergeWithPreviousBlockCommand : public QUndoCommand
{
public:
    MergeWithPreviousBlockCommand(TreeItem *block, const QString &text, TreeItem *target, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~MergeWithPreviousBlockCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;

    TreeItem *m_originalTargetParent;
    int m_originalTargetRow;
    TreeItem *m_originalTarget;

    TreeItem *m_originalBlockParent;
    int m_originalBlockRow;
    TreeItem *m_originalBlock;

    QList<TreeItem *> m_newBlocks;
    int m_focusCursorPos;
};

#endif // MERGEWITHPREVIOUSBLOCK_H
