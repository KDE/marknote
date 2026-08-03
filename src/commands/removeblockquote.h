// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMOVEBLOCKQUOTE_H
#define REMOVEBLOCKQUOTE_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class RemoveBlockquoteCommand : public QUndoCommand
{
public:
    RemoveBlockquoteCommand(TreeItem *blockquote, TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent = nullptr);
    ~RemoveBlockquoteCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_blockquote;
    TreeItem *m_block;
    TreeItem *m_parentBlock;
    MDTreeModel *m_model;

    int m_blockquoteRow = -1;
    int m_childCount = 0;
    int m_cursorPosition = -1;
};

#endif // REMOVEBLOCKQUOTE_H
