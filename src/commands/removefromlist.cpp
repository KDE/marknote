// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "removefromlist.h"
#include <QTimer>

RemoveFromListCommand::RemoveFromListCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_listItem(block->parent())
    , m_list(m_listItem->parent())
    , m_parentBlock(m_list->parent())
    , m_model(model)
    , m_cursorPosition(cursorPosition)
{
    m_listRow = m_list->row();
    m_listItemRow = m_listItem->row();
    m_itemsCount = m_listItem->childCount();
    m_nextSiblingsCount = m_list->childCount() - m_listItemRow - 1;

    if (m_nextSiblingsCount > 0) {
        m_newList = TreeItem::createTreeItem(MDOptions::ElementType::List);
        delete m_newList->removeChild(0);
        m_ownsNewList = true;
        m_newListCreated = true;
    }
}

RemoveFromListCommand::~RemoveFromListCommand()
{
}

void RemoveFromListCommand::undo()
{
    if (m_ownsList) {
        m_model->insertItem(m_parentBlock, m_listRow, m_list);
        m_ownsList = false;
    }

    if (m_ownsListItem) {
        m_model->insertItem(m_list, m_listItemRow, m_listItem);
        m_ownsListItem = false;
    }

    QTimer::singleShot(0, [this]() {
        int itemsStartRow = m_listRow + 1;

        if (m_itemsCount > 0) {
            m_model->moveItems(m_parentBlock, itemsStartRow, m_itemsCount, m_listItem, 0);
        }

        QTimer::singleShot(0, [this, itemsStartRow]() {
            if (m_nextSiblingsCount > 0) {
                m_model->moveItems(m_newList, 0, m_nextSiblingsCount, m_list, m_listItemRow + 1);
                m_model->takeItem(m_parentBlock, itemsStartRow);
                m_ownsNewList = true;
            }

            m_model->updateListNumbers(m_list, true);
            m_model->requestFocus(m_block, m_cursorPosition);
        });
    });
}

void RemoveFromListCommand::redo()
{
    int insertRow = m_listRow + 1;

    if (m_nextSiblingsCount > 0) {
        if (m_newListCreated) {
            m_model->insertItem(m_parentBlock, insertRow, m_newList);
            m_ownsNewList = false;
        }
        m_model->moveItems(m_list, m_listItemRow + 1, m_nextSiblingsCount, m_newList, 0);
    }

    if (m_itemsCount > 0) {
        m_model->moveItems(m_listItem, 0, m_itemsCount, m_parentBlock, insertRow);
    }

    QTimer::singleShot(0, [this]() {
        m_model->takeItem(m_list, m_listItemRow);
        m_ownsListItem = true;

        if (m_list->childCount() == 0) {
            m_model->takeItem(m_parentBlock, m_listRow);
            m_ownsList = true;
        }

        if (!m_ownsList) {
            m_model->updateListNumbers(m_list, true);
        }

        if (!m_ownsNewList && m_newList) {
            m_model->updateListNumbers(m_newList, true);
        }

        m_model->requestFocus(m_block, m_cursorPosition);
    });
}
