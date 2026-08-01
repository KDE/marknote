// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "insertparagraphbelow.h"

InsertParagraphBelowCommand::InsertParagraphBelowCommand(TreeItem *block, const QString &text, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_originalBlock(block)
    , m_text(text)
{
    m_parent = block->parent();
    m_row = block->row() + 1;
    m_newBlock = TreeItem::createTreeItem(MDOptions::ElementType::Paragraph, text);
}

InsertParagraphBelowCommand::~InsertParagraphBelowCommand()
{
}

void InsertParagraphBelowCommand::undo()
{
    m_model->takeItem(m_parent, m_row);
    m_model->requestFocus(m_originalBlock, -1);
}

void InsertParagraphBelowCommand::redo()
{
    m_model->insertItem(m_parent, m_row, m_newBlock);
    m_model->requestFocus(m_newBlock, m_text.length());
}
