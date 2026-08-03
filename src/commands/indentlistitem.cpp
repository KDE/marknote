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
            m_targetSubList = TreeItem::createTreeItem(MDOptions::ElementType::List);
            delete m_targetSubList->removeChild(0);
            m_targetSubListCreated = true;
            m_ownsTargetSubList = true;
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

    m_model->moveItem(m_listItem, m_curList, m_listItemRow);

    if (m_targetSubListCreated) {
        m_model->takeItem(m_prevListItem, m_targetSubList->row());
        m_ownsTargetSubList = true;
    } else {
        m_model->updateListNumbers(m_targetSubList, true);
    }

    m_model->updateListNumbers(m_curList, true);
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}

void IndentListItemCommand::redo()
{
    if (m_listItemRow <= 0) {
        return;
    }

    if (m_targetSubListCreated) {
        m_model->insertItem(m_prevListItem, m_prevListItem->childCount(), m_targetSubList);
        m_ownsTargetSubList = false;
    }

    m_model->moveItem(m_listItem, m_targetSubList, m_targetSubList->childCount());

    m_model->updateListNumbers(m_curList, true);
    m_model->updateListNumbers(m_targetSubList, true);
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}
