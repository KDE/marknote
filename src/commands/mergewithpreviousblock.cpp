// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mergewithpreviousblock.h"
#include <md4qt/parser.h>

using namespace Qt::StringLiterals;

MergeWithPreviousBlockCommand::MergeWithPreviousBlockCommand(TreeItem *block, const QString &text, TreeItem *target, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_originalTargetParent(target->parent())
    , m_originalTargetRow(target->row())
    , m_originalTarget(target)
    , m_originalBlockParent(block->parent())
    , m_originalBlockRow(block->row())
    , m_originalBlock(block)
{
    QString targetMd = target->data().value(u"md"_s).toString();

    m_focusCursorPos = targetMd.length();

    QString combinedMd = targetMd + text;

    m_newBlocks = TreeItem::fromMarkdown(combinedMd);
    if (m_newBlocks.empty()) {
        m_newBlocks.append(TreeItem::createTreeItem(MDOptions::ElementType::Paragraph));
    }
}

MergeWithPreviousBlockCommand::~MergeWithPreviousBlockCommand()
{
}

void MergeWithPreviousBlockCommand::undo()
{
    for (int i = 0; i < m_newBlocks.size(); i++) {
        m_model->takeItem(m_originalTargetParent, m_originalTargetRow);
    }

    m_model->insertItem(m_originalTargetParent, m_originalTargetRow, m_originalTarget);
    m_model->insertItem(m_originalBlockParent, m_originalBlockRow, m_originalBlock);

    m_model->requestFocus(m_originalBlock, 0);
}

void MergeWithPreviousBlockCommand::redo()
{
    m_model->takeItem(m_originalBlockParent, m_originalBlockRow);
    m_model->takeItem(m_originalTargetParent, m_originalTargetRow);

    int offset = m_originalTargetRow;
    for (auto block : m_newBlocks) {
        m_model->insertItem(m_originalTargetParent, offset++, block);
    }

    m_model->requestFocus(m_newBlocks.first(), m_focusCursorPos);
}
