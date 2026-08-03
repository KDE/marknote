// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "transformtoblockquote.h"

using namespace Qt::StringLiterals;

TransformToBlockquoteCommand::TransformToBlockquoteCommand(TreeItem *block, int level, const QString &text, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_originalParent(block->parent())
    , m_originalRow(block->row())
    , m_originalBlock(block)
    , m_level(level)
    , m_text(text)
    , m_outerBlockquote(nullptr)
{
    m_oldText = block->data().value(u"md"_s).toString();

    m_outerBlockquote = TreeItem::createTreeItem(MDOptions::ElementType::Blockquote);
    delete m_outerBlockquote->removeChild(0);

    TreeItem *inner = m_outerBlockquote;
    for (int i = 1; i < m_level; ++i) {
        TreeItem *b = TreeItem::createTreeItem(MDOptions::ElementType::Blockquote);
        delete b->removeChild(0);
        inner->appendChild(b);
        inner = b;
    }
}

TransformToBlockquoteCommand::~TransformToBlockquoteCommand()
{
}

void TransformToBlockquoteCommand::undo()
{
    qDebug() << "Transform to blockquote undo";
    m_model->takeItem(m_originalParent, m_originalRow);

    m_originalBlock->parent()->removeChild(m_originalBlock->row());

    m_originalBlock->setUnparsedMarkdown(m_oldText);
    m_model->insertItem(m_originalParent, m_originalRow, m_originalBlock);
    m_model->requestFocus(m_originalBlock, m_level);
}

void TransformToBlockquoteCommand::redo()
{
    qDebug() << "Transform to blockquote redo";
    m_model->takeItem(m_originalParent, m_originalRow);
    m_originalBlock->setUnparsedMarkdown(m_text);

    TreeItem *inner = m_outerBlockquote;
    for (int i = 1; i < m_level; ++i) {
        inner = inner->child(0);
    }
    inner->appendChild(m_originalBlock);

    m_model->insertItem(m_originalParent, m_originalRow, m_outerBlockquote);
    m_model->requestFocus(m_originalBlock, 0);
}
