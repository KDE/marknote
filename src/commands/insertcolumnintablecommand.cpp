// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "insertcolumnintablecommand.h"

using namespace Qt::StringLiterals;

InsertColumnInTableCommand::InsertColumnInTableCommand(TreeItem *block, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_col(-1)
{
}

InsertColumnInTableCommand::~InsertColumnInTableCommand()
{
}

void InsertColumnInTableCommand::undo()
{
    if (m_col >= 0) {
        m_model->removeColFromTable(m_block, m_col);
        m_model->requestFocusOnTable(m_block, 0, qMax(0, m_col - 1), 0);
    }
}

void InsertColumnInTableCommand::redo()
{
    m_model->appendColInTable(m_block);
    m_col = m_block->data().value(u"columnCount"_s).toInt() - 1;
    m_model->requestFocusOnTable(m_block, 0, m_col, 0);
}
