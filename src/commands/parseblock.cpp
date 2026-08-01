// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "parseblock.h"

ParseBlockCommand::ParseBlockCommand(TreeItem *block, const QString &text, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_originalParent(block->parent())
    , m_originalRow(block->row())
    , m_originalBlock(block)
    , m_firstTime(true)
{
    m_newBlocks = TreeItem::fromMarkdown(text);
    if (m_newBlocks.empty()) {
        m_newBlocks.append(TreeItem::createTreeItem(MDOptions::ElementType::Paragraph));
    }
}

ParseBlockCommand::~ParseBlockCommand()
{
}

void ParseBlockCommand::undo()
{
    for (int i = 0; i < m_newBlocks.size(); i++) {
        m_model->takeItem(m_originalParent, m_originalRow);
    }

    m_model->insertItem(m_originalParent, m_originalRow, m_originalBlock);

    m_model->requestFocus(m_originalBlock);
}

void ParseBlockCommand::redo()
{
    m_model->takeItem(m_originalParent, m_originalRow);

    int offset = m_originalRow;
    for (auto block : m_newBlocks) {
        m_model->insertItem(m_originalParent, offset++, block);
    }

    if (!m_firstTime) {
        m_model->requestFocus(m_newBlocks.first());
    } else {
        m_firstTime = false;
    }
}
