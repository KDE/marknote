// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOVEOUTSIDEBLOCKQUOTECOMMAND_H
#define MOVEOUTSIDEBLOCKQUOTECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class MoveOutsideBlockquoteCommand : public QUndoCommand
{
public:
    explicit MoveOutsideBlockquoteCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent = nullptr);
    ~MoveOutsideBlockquoteCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    TreeItem *m_blockquote;
    TreeItem *m_parentBlock;
    MDTreeModel *m_model;
    int m_cursorPosition;

    int m_blockquoteRow;
    int m_originalRow;
};

#endif // MOVEOUTSIDEBLOCKQUOTECOMMAND_H
