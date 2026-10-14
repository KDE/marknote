// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "indentlistitem.h"

using namespace Qt::StringLiterals;

IndentListItemCommand::IndentListItemCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_listItem(block->parent())
    , m_curList(m_listItem->parent())
    , m_model(model)
    , m_cursorPosition(cursorPosition)
{
    m_listItemRow = m_listItem->row();
    if (m_listItemRow > 0) {
        m_prevListItem = m_curList->child(m_listItemRow - 1);

        if (m_prevListItem->childCount() > 0 && m_prevListItem->children().back()->type() == MDOptions::ElementType::List) {
            m_targetSubList = m_prevListItem->children().back();
            m_targetSubListCreated = false;
            m_ownsTargetSubList = false;
        } else {
            m_targetSubList = TreeItem::createTreeItem(MDOptions::ElementType::List, block->doc());
            delete m_targetSubList->removeChild(0);
            m_targetSubListCreated = true;
            m_ownsTargetSubList = true;
        }

        auto curMdItem = m_listItem->itemAs<MD::ListItem>();
        if (curMdItem) {
            m_originalListType = curMdItem->listType();
            m_originalSymbol = curMdItem->symbol();
            m_originalIsTaskList = curMdItem->isTaskList();
        }

        if (!m_targetSubListCreated && m_targetSubList && m_targetSubList->childCount() > 0) {
            auto targetMdItem = m_targetSubList->child(0)->itemAs<MD::ListItem>();
            if (targetMdItem) {
                m_targetListType = targetMdItem->listType();
                m_targetSymbol = targetMdItem->symbol();
                m_targetIsTaskList = targetMdItem->isTaskList();
                m_shouldChangeType = true;
            }
        }
    }
}

IndentListItemCommand::~IndentListItemCommand()
{
}

void IndentListItemCommand::undo()
{
    if (m_listItemRow <= 0) {
        return;
    }

    if (m_shouldChangeType) {
        auto mdListItem = m_listItem->itemAs<MD::ListItem>();
        if (mdListItem) {
            mdListItem->setListType(m_originalListType);
            mdListItem->setSymbol(m_originalSymbol);
            mdListItem->setTaskList(m_originalIsTaskList);
        }
    }

    m_model->moveItem(m_listItem, m_curList, m_listItemRow);

    if (m_targetSubListCreated) {
        m_model->takeItem(m_prevListItem, m_targetSubList->row());
        m_ownsTargetSubList = true;
    } else {
        m_model->updateListNumbers(m_targetSubList, false);
    }

    m_model->updateListNumbers(m_curList, false);
    if (m_shouldChangeType) {
        m_model->childModified(m_curList, m_listItem->row(), m_listItem->row());
    }
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}

void IndentListItemCommand::redo()
{
    if (m_listItemRow <= 0) {
        return;
    }

    if (m_shouldChangeType) {
        auto mdListItem = m_listItem->itemAs<MD::ListItem>();
        if (mdListItem) {
            mdListItem->setListType(m_targetListType);
            mdListItem->setSymbol(m_targetSymbol);
            mdListItem->setTaskList(m_targetIsTaskList);
            if (m_targetIsTaskList && !m_originalIsTaskList) {
                mdListItem->setChecked(false);
            }
        }
    }

    if (m_targetSubListCreated) {
        m_model->insertItem(m_prevListItem, m_prevListItem->childCount(), m_targetSubList);
        m_ownsTargetSubList = false;
    }

    m_model->moveItem(m_listItem, m_targetSubList, m_targetSubList->childCount());

    m_model->updateListNumbers(m_curList, false);
    m_model->updateListNumbers(m_targetSubList, m_targetSubListCreated);
    if (m_shouldChangeType) {
        m_model->childModified(m_targetSubList, m_listItem->row(), m_listItem->row());
    }
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}
