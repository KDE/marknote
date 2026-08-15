// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "moveblockcommand.h"

MoveBlockCommand::MoveBlockCommand(TreeItem *sourceBlock, TreeItem *targetParent, int targetIndex, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_sourceBlock(sourceBlock)
    , m_targetParent(targetParent)
    , m_targetIndex(targetIndex)
    , m_model(model)
{
    if (!m_sourceBlock || !m_targetParent || !m_model) {
        m_isValid = false;
        return;
    }

    m_oldParent = m_sourceBlock->parent();
    m_oldIndex = m_sourceBlock->row();

    if (!m_oldParent) {
        m_isValid = false;
        return;
    }

    if (m_targetParent == m_sourceBlock || m_targetParent->isDescendantOf(m_sourceBlock)) {
        m_isValid = false;
        return;
    }
}

MoveBlockCommand::~MoveBlockCommand()
{
}

void MoveBlockCommand::redo()
{
    if (!m_isValid)
        return;

    TreeItem *current = m_oldParent;
    TreeItem *topEmptyAncestor = nullptr;

    while (current && current->type() != MDOptions::ElementType::Document) {
        if (current->childCount() == 1 && current != m_targetParent && !m_targetParent->isDescendantOf(current)) {
            topEmptyAncestor = current;
            current = current->parent();
        } else {
            break;
        }
    }

    if (topEmptyAncestor) {
        m_removedEmptySubtreeParent = topEmptyAncestor->parent();
        m_removedEmptySubtreeIndex = topEmptyAncestor->row();

        m_removedEmptySubtree = m_model->takeItem(m_removedEmptySubtreeParent, m_removedEmptySubtreeIndex);

        m_sourceBlock = m_oldParent->removeChild(m_oldIndex);
    } else {
        m_sourceBlock = m_model->takeItem(m_oldParent, m_oldIndex);
    }

    TreeItem *itemToInsert = m_sourceBlock;

    if (m_targetParent->type() == MDOptions::ElementType::List && m_sourceBlock->type() != MDOptions::ElementType::ListItem) {
        const auto prevListType = m_targetParent->child(0)->item().dynamicCast<MD::ListItem>()->listType();

        if (!m_wrappedListItem) {
            m_wrappedListItem = TreeItem::createTreeItem(MDOptions::ElementType::ListItem);
            m_wrappedListItem->removeChild(0);
        }

        m_wrappedListItem->item().dynamicCast<MD::ListItem>()->setListType(prevListType);

        m_wrappedListItem->appendChild(m_sourceBlock);
        itemToInsert = m_wrappedListItem;
    } else if (m_sourceBlock->type() == MDOptions::ElementType::ListItem && m_targetParent->type() != MDOptions::ElementType::List) {
        if (!m_wrappedList) {
            m_wrappedList = TreeItem::createTreeItem(MDOptions::ElementType::List);
            m_wrappedList->removeChild(0);
        }
        m_wrappedList->appendChild(m_sourceBlock);
        itemToInsert = m_wrappedList;
    }

    int actualTargetIndex = m_targetIndex;
    if (topEmptyAncestor) {
        if (m_removedEmptySubtreeParent == m_targetParent && m_removedEmptySubtreeIndex < m_targetIndex) {
            actualTargetIndex--;
        }
    } else {
        if (m_oldParent == m_targetParent && m_oldIndex < m_targetIndex) {
            actualTargetIndex--;
        }
    }

    m_model->insertItem(m_targetParent, actualTargetIndex, itemToInsert);

    if (m_oldParent->type() == MDOptions::ElementType::List && !topEmptyAncestor) {
        m_model->updateListNumbers(m_oldParent, false);
    } else if (topEmptyAncestor && m_removedEmptySubtreeParent && m_removedEmptySubtreeParent->type() == MDOptions::ElementType::List) {
        m_model->updateListNumbers(m_removedEmptySubtreeParent, false);
    }

    if (m_targetParent->type() == MDOptions::ElementType::List) {
        bool shouldResetList = actualTargetIndex == 0;
        m_model->updateListNumbers(m_targetParent, shouldResetList);
    }
}

void MoveBlockCommand::undo()
{
    if (!m_isValid)
        return;

    TreeItem *insertedItem = m_sourceBlock;
    if (m_wrappedListItem && m_wrappedListItem->parent()) {
        insertedItem = m_wrappedListItem;
    } else if (m_wrappedList && m_wrappedList->parent()) {
        insertedItem = m_wrappedList;
    }

    int insertedIndex = insertedItem->row();
    m_model->takeItem(m_targetParent, insertedIndex);

    if (insertedItem == m_wrappedListItem) {
        m_wrappedListItem->removeChild(0);
    } else if (insertedItem == m_wrappedList) {
        m_wrappedList->removeChild(0);
    }

    if (m_removedEmptySubtree) {
        m_oldParent->insertChild(m_oldIndex, m_sourceBlock);
        m_model->insertItem(m_removedEmptySubtreeParent, m_removedEmptySubtreeIndex, m_removedEmptySubtree);
    } else {
        m_model->insertItem(m_oldParent, m_oldIndex, m_sourceBlock);
    }

    if (m_oldParent->type() == MDOptions::ElementType::List) {
        m_model->updateListNumbers(m_oldParent, false);
    } else if (m_removedEmptySubtreeParent && m_removedEmptySubtreeParent->type() == MDOptions::ElementType::List) {
        m_model->updateListNumbers(m_removedEmptySubtreeParent, false);
    }

    if (m_targetParent->type() == MDOptions::ElementType::List) {
        m_model->updateListNumbers(m_targetParent, false);
    }
}
