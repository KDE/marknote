// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "splitlistitem.h"
#include <md4qt/doc.h>

using namespace Qt::StringLiterals;

/*
List structure for reference:

List:
    ListItem:
        Paragraph (This is the *block pointer)
*/

SplitListItemCommand::SplitListItemCommand(TreeItem *block, const QString &text, int splitIndex, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_parent(block->parent())
    , m_row(block->row())
    , m_text(text)
    , m_splitIndex(splitIndex)
    , m_model(model)
    , m_originalBlock(block)
{
    const QString leftText = m_text.left(m_splitIndex);
    const QString rightText = m_text.mid(m_splitIndex);

    m_leftBlocks = TreeItem::fromMarkdown(leftText);
    m_rightBlocks = TreeItem::fromMarkdown(rightText);

    m_ownsNewBlocks = true;
}

SplitListItemCommand::~SplitListItemCommand()
{
}

void SplitListItemCommand::undo()
{
    m_removedList = m_model->takeItem(m_parent->parent(), m_parent->row() + 1);

    for (int offset = 0; offset < m_leftBlocks.size(); offset++) {
        m_model->takeItem(m_parent, m_row);
    }

    m_model->insertItem(m_parent, m_row, m_originalBlock);

    m_model->requestFocus(m_originalBlock, m_splitIndex);
}

void SplitListItemCommand::redo()
{
    m_model->takeItem(m_parent, m_row);

    int offset = m_row;
    for (auto block : m_leftBlocks) {
        m_model->insertItem(m_parent, offset++, block);
    }

    if (!m_removedList) {
        auto curListItem = m_parent->itemAs<MD::ListItem>();
        auto newListItem = QSharedPointer<MD::ListItem>::create();

        newListItem->setListType(curListItem->listType());
        if (curListItem->listType() == MD::ListItem::ListType::Ordered) {
            newListItem->setStartNumber(curListItem->startNumber() + 1);
        }

        if (curListItem->isTaskList()) {
            newListItem->setTaskList(true);
            newListItem->setChecked(false);
        }

        m_removedList = TreeItem::buildTree(newListItem);
        if (m_removedList->childCount() > 0) {
            m_removedList->takeChildren(0, m_removedList->childCount());
        }

        m_removedList->insertChildren(0, m_rightBlocks);
    }

    m_model->insertItem(m_parent->parent(), m_parent->row() + 1, m_removedList);
    m_model->requestFocus(m_rightBlocks.first(), 0);
}