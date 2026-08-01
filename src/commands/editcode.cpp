// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "editcode.h"

EditCodeCommand::EditCodeCommand(TreeItem *block,
                                 const QString &oldText,
                                 const QString &newText,
                                 int oldCursorPosition,
                                 int newCursorPosition,
                                 MDTreeModel *model,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_block(block)
    , m_oldText(oldText)
    , m_newText(newText)
    , m_oldCursorPosition(oldCursorPosition)
    , m_newCursorPosition(newCursorPosition)
    , m_isFirstTime(true)
{
}

EditCodeCommand::~EditCodeCommand()
{
}

void EditCodeCommand::undo()
{
    m_model->setItemCode(m_block, m_oldText);
    m_model->requestFocus(m_block, m_oldCursorPosition);
}

void EditCodeCommand::redo()
{
    m_model->setItemCode(m_block, m_newText);

    if (!m_isFirstTime) {
        m_model->requestFocus(m_block, m_newCursorPosition);
    }

    m_isFirstTime = false;
}
