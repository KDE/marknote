// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "edittext.h"

EditTextCommand::EditTextCommand(TreeItem *block,
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

EditTextCommand::~EditTextCommand()
{
}

void EditTextCommand::undo()
{
    m_block->setUnparsedMarkdown(m_oldText);
    m_model->childModified(m_block->parent(), m_block->row(), m_block->row());
    m_model->requestFocus(m_block, m_oldCursorPosition);
}

void EditTextCommand::redo()
{
    qDebug() << "Redoing edit text: " << m_newText;
    m_block->setUnparsedMarkdown(m_newText);
    m_model->childModified(m_block->parent(), m_block->row(), m_block->row());

    if (!m_isFirstTime) {
        m_model->requestFocus(m_block, m_newCursorPosition);
    }

    m_isFirstTime = false;
}
