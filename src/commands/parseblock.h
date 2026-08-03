// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PARSEBLOCK_H
#define PARSEBLOCK_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class ParseBlockCommand : public QUndoCommand
{
public:
    ParseBlockCommand(TreeItem *block, const QString &text, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~ParseBlockCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_originalParent;
    int m_originalRow;
    TreeItem *m_originalBlock;
    QList<TreeItem *> m_newBlocks;
    bool m_firstTime;
};

#endif // PARSEBLOCK_H
