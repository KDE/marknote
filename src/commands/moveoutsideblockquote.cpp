// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "moveoutsideblockquote.h"

MoveOutsideBlockquoteCommand::MoveOutsideBlockquoteCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_blockquote(block->parent())
    , m_parentBlock(m_blockquote->parent())
    , m_model(model)
    , m_cursorPosition(cursorPosition)
{
    m_blockquoteRow = m_blockquote->row();
    m_originalRow = m_block->row();
}

MoveOutsideBlockquoteCommand::~MoveOutsideBlockquoteCommand()
{
}

void MoveOutsideBlockquoteCommand::undo()
{
    m_model->moveItem(m_block, m_blockquote, m_originalRow);
    m_model->requestFocus(m_block, m_cursorPosition);
}

void MoveOutsideBlockquoteCommand::redo()
{
    m_model->moveItem(m_block, m_parentBlock, m_blockquoteRow + 1);
    m_model->requestFocus(m_block, m_cursorPosition);
}
