// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "removeblockquote.h"
#include <QTimer>

RemoveBlockquoteCommand::RemoveBlockquoteCommand(TreeItem *blockquote, TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_blockquote(blockquote)
    , m_block(block)
    , m_parentBlock(blockquote->parent())
    , m_model(model)
    , m_cursorPosition(cursorPosition)
{
    m_blockquoteRow = m_blockquote->row();
    m_childCount = m_blockquote->childCount();
}

RemoveBlockquoteCommand::~RemoveBlockquoteCommand()
{
}

void RemoveBlockquoteCommand::undo()
{
    qDebug() << "Remove blockquote undo";
    m_model->insertItem(m_parentBlock, m_blockquoteRow, m_blockquote);

    QTimer::singleShot(0, [this]() {
        if (m_childCount > 0) {
            m_model->moveItems(m_parentBlock, m_blockquoteRow + 1, m_childCount, m_blockquote, 0);
        }

        m_model->requestFocus(m_block, m_cursorPosition);
    });
}

void RemoveBlockquoteCommand::redo()
{
    qDebug() << "Remove blockquote redo";
    if (m_childCount > 0) {
        m_model->moveItems(m_blockquote, 0, m_childCount, m_parentBlock, m_blockquoteRow);
    }

    m_model->takeItem(m_parentBlock, m_blockquote->row());
    m_model->requestFocus(m_block, m_cursorPosition);
}
