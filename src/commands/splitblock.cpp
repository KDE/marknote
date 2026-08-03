// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "splitblock.h"
#include <md4qt/parser.h>

using namespace Qt::StringLiterals;

SplitBlockCommand::SplitBlockCommand(TreeItem *block, const QString &text, int splitIndex, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_parent(block->parent())
    , m_text(text)
    , m_splitIndex(splitIndex)
    , m_row(block->row())
    , m_originalBlock(block)
{
    const QString leftText = m_text.left(m_splitIndex);
    const QString rightText = m_text.mid(m_splitIndex);

    m_leftBlocks = TreeItem::fromMarkdown(leftText);
    if (m_leftBlocks.empty()) {
        m_leftBlocks.append(TreeItem::createTreeItem(MDOptions::ElementType::Paragraph));
    }

    m_rightBlocks = TreeItem::fromMarkdown(rightText);
    if (m_rightBlocks.empty()) {
        m_rightBlocks.append(TreeItem::createTreeItem(MDOptions::ElementType::Paragraph));
    }

    m_ownsNewBlocks = true;
}

SplitBlockCommand::~SplitBlockCommand()
{
}

void SplitBlockCommand::undo()
{
    for (int offset = 0; offset < m_leftBlocks.size() + m_rightBlocks.size(); offset++) {
        m_model->takeItem(m_parent, m_row);
    }
    m_ownsNewBlocks = true;

    m_model->insertItem(m_parent, m_row, m_originalBlock);
    m_ownsOriginalBlock = false;

    m_model->requestFocus(m_originalBlock, m_splitIndex);
}

void SplitBlockCommand::redo()
{
    m_model->takeItem(m_parent, m_row);
    m_ownsOriginalBlock = true;

    int offset = m_row;
    for (auto block : m_leftBlocks) {
        m_model->insertItem(m_parent, offset++, block);
    }
    for (auto block : m_rightBlocks) {
        m_model->insertItem(m_parent, offset++, block);
    }
    m_ownsNewBlocks = false;

    m_model->requestFocus(m_rightBlocks.first(), 0);
}