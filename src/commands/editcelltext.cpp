// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "editcelltext.h"

EditCellTextCommand::EditCellTextCommand(TreeItem *block,
                                         int row,
                                         int column,
                                         const QString &oldText,
                                         const QString &newText,
                                         int oldCursorPosition,
                                         int newCursorPosition,
                                         MDTreeModel *model,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_row(row)
    , m_column(column)
    , m_oldText(oldText)
    , m_newText(newText)
    , m_oldCursorPosition(oldCursorPosition)
    , m_newCursorPosition(newCursorPosition)
    , m_model(model)
    , m_isFirstTime(true)
{
}

EditCellTextCommand::~EditCellTextCommand()
{
}

void EditCellTextCommand::undo()
{
    m_model->setItemTableMD(m_block, m_row, m_column, m_oldText);
    m_model->setTableCellMD(m_block, m_row, m_column, m_oldText);
    m_model->requestFocusOnTable(m_block, m_row, m_column, m_oldCursorPosition);
}

void EditCellTextCommand::redo()
{
    m_model->setItemTableMD(m_block, m_row, m_column, m_newText);
    m_model->setTableCellMD(m_block, m_row, m_column, m_newText);

    if (!m_isFirstTime) {
        m_model->requestFocusOnTable(m_block, m_row, m_column, m_newCursorPosition);
    }
    m_isFirstTime = false;
}
