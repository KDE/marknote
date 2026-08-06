// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "insertrowintablecommand.h"

using namespace Qt::StringLiterals;

InsertRowInTableCommand::InsertRowInTableCommand(TreeItem *block, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_row(-1)
{
}

InsertRowInTableCommand::~InsertRowInTableCommand()
{
}

void InsertRowInTableCommand::undo()
{
    if (m_row >= 0) {
        m_model->removeRowFromTable(m_block, m_row);
        m_model->requestFocusOnTable(m_block, qMax(0, m_row - 1), 0, 0);
    }
}

void InsertRowInTableCommand::redo()
{
    m_model->appendRowInTable(m_block);
    m_row = m_block->data().value(u"rowCount"_s).toInt() - 1;
    m_model->requestFocusOnTable(m_block, m_row, 0, 0);
}
