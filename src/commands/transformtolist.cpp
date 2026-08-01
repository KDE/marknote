// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "transformtolist.h"

using namespace Qt::StringLiterals;

TransformToListCommand::TransformToListCommand(TreeItem *block, bool isOrdered, int startNumber, const QString &text, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_originalParent(block->parent())
    , m_originalRow(block->row())
    , m_originalBlock(block)
    , m_isOrdered(isOrdered)
    , m_startNumber(startNumber)
    , m_text(text)
    , m_outerList(nullptr)
{
    m_oldText = block->data().value(u"md"_s).toString();

    m_outerList = TreeItem::createTreeItem(MDOptions::ElementType::List);
    TreeItem *listItem = m_outerList->child(0);
    delete listItem->removeChild(0); // Remove the auto-generated paragraph

    auto mdListItem = listItem->itemAs<MD::ListItem>();
    if (m_isOrdered) {
        mdListItem->setListType(MD::ListItem::Ordered);
        mdListItem->setStartNumber(m_startNumber);
    } else {
        mdListItem->setListType(MD::ListItem::Unordered);
    }
}

TransformToListCommand::~TransformToListCommand()
{
}

void TransformToListCommand::undo()
{
    m_model->takeItem(m_originalParent, m_originalRow); // Takes m_outerList

    m_originalBlock->parent()->removeChild(m_originalBlock->row());

    m_model->setItemMD(m_originalBlock, m_oldText);
    m_model->insertItem(m_originalParent, m_originalRow, m_originalBlock);

    int offset = m_oldText.length() - m_text.length();
    if (offset < 0)
        offset = 0;
    m_model->requestFocus(m_originalBlock, offset);
}

void TransformToListCommand::redo()
{
    m_model->takeItem(m_originalParent, m_originalRow); // Takes m_originalBlock
    m_model->setItemMD(m_originalBlock, m_text);

    TreeItem *listItem = m_outerList->child(0);
    listItem->appendChild(m_originalBlock);

    m_model->insertItem(m_originalParent, m_originalRow, m_outerList);
    m_model->requestFocus(m_originalBlock, 0);
}
