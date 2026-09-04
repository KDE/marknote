// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "changelisttypecommand.h"

using namespace Qt::StringLiterals;

ChangeListTypeCommand::ChangeListTypeCommand(TreeItem *block, MDOptions::ListType targetType, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_targetType(targetType)
    , m_listNode(nullptr)
    , m_isValid(false)
{
    if (!m_block || !m_model) {
        return;
    }

    TreeItem *listItemNode = m_block->parent();
    if (!listItemNode || listItemNode->type() != MDOptions::ElementType::ListItem) {
        return;
    }

    m_listNode = listItemNode->parent();
    if (!m_listNode || m_listNode->type() != MDOptions::ElementType::List) {
        return;
    }

    for (int i = 0; i < m_listNode->childCount(); ++i) {
        TreeItem *child = m_listNode->child(i);
        if (child->type() == MDOptions::ElementType::ListItem) {
            auto mdListItem = child->itemAs<MD::ListItem>();
            if (mdListItem) {
                OriginalState state;
                state.listItem = child;
                state.listType = mdListItem->listType();
                state.isTaskList = mdListItem->isTaskList();
                m_originalStates.append(state);
            }
        }
    }

    m_isValid = true;
}

ChangeListTypeCommand::~ChangeListTypeCommand()
{
}

void ChangeListTypeCommand::undo()
{
    if (!m_isValid || !m_listNode) {
        return;
    }

    for (const OriginalState &state : m_originalStates) {
        auto mdListItem = state.listItem->itemAs<MD::ListItem>();
        if (mdListItem) {
            mdListItem->setListType(state.listType);
            mdListItem->setTaskList(state.isTaskList);
        }
    }

    if (m_listNode->childCount() > 0) {
        m_model->childModified(m_listNode, 0, m_listNode->childCount() - 1);
    }
}

void ChangeListTypeCommand::redo()
{
    if (!m_isValid || !m_listNode) {
        return;
    }

    for (int i = 0; i < m_listNode->childCount(); ++i) {
        TreeItem *child = m_listNode->child(i);
        if (child->type() == MDOptions::ElementType::ListItem) {
            auto mdListItem = child->itemAs<MD::ListItem>();
            if (mdListItem) {
                if (m_targetType == MDOptions::OrderedList) {
                    mdListItem->setListType(MD::ListItem::Ordered);
                    mdListItem->setTaskList(false);
                } else if (m_targetType == MDOptions::UnorderedList) {
                    mdListItem->setListType(MD::ListItem::Unordered);
                    mdListItem->setTaskList(false);
                } else if (m_targetType == MDOptions::TaskList) {
                    mdListItem->setTaskList(true);
                }
            }
        }
    }

    if (m_targetType == MDOptions::OrderedList) {
        m_model->updateListNumbers(m_listNode, true);
    }

    if (m_listNode->childCount() > 0) {
        m_model->childModified(m_listNode, 0, m_listNode->childCount() - 1);
    }
}
