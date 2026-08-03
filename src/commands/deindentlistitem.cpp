// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deindentlistitem.h"

using namespace Qt::StringLiterals;

/*
List structure for reference:

List: (targetList)
    ListItem:
        Paragraph
    ListItem:
        Paragraph
        List: (curList)
            ListItem:
                Paragraph
            ListItem: (listItem) <--- This will be de-indented
                Paragraph
                List:
                    ListItem:
                        Paragraph
                    ListItem:
                        Paragraph
            ListItem:
                Paragraph
            ListItem:
                Paragraph
*/

DeIndentListItemCommand::DeIndentListItemCommand(TreeItem *block, MDTreeModel *model, int cursorPosition, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_listItem(block->parent())
    , m_curList(m_listItem->parent())
    , m_targetList(m_curList->parent()->parent())
    , m_row(block->row())
    , m_model(model)
    , m_cursorPosition(cursorPosition)
{
    m_listItemRow = m_listItem->row();
    m_parentListItemRow = m_curList->parent()->row();
    m_nextSiblingCount = m_curList->childCount() - m_listItemRow - 1;
    m_curListParent = m_curList->parent();
    m_curListRow = m_curList->row();

    if (m_listItem->childCount() > 0 && m_listItem->children().back()->type() == MDOptions::ElementType::List) {
        m_listItemSubList = m_listItem->children().back();
        m_subListCreated = false;
        m_ownsSubList = false;
    } else {
        m_listItemSubList = TreeItem::createTreeItem(MDOptions::ElementType::List);
        delete m_listItemSubList->removeChild(0);
        m_subListCreated = true;
        m_ownsSubList = true;
    }
}

DeIndentListItemCommand::~DeIndentListItemCommand()
{
}

void DeIndentListItemCommand::undo()
{
    if (m_ownsCurList) {
        m_model->insertItem(m_curListParent, m_curListRow, m_curList);
        m_ownsCurList = false;
    }

    if (m_nextSiblingCount > 0) {
        int startIndex = m_listItemSubList->childCount() - m_nextSiblingCount;
        m_model->moveItems(m_listItemSubList, startIndex, m_nextSiblingCount, m_curList, m_listItemRow);

        if (m_subListCreated) {
            m_model->takeItem(m_listItem, m_listItemSubList->row());
            m_ownsSubList = true;
        }
    }

    m_model->moveItem(m_listItem, m_curList, m_listItemRow);
    m_model->updateListNumbers(m_curList, true);
    m_model->updateListNumbers(m_targetList);
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}

void DeIndentListItemCommand::redo()
{
    m_model->moveItem(m_listItem, m_targetList, m_parentListItemRow + 1);

    if (m_curList->childCount() == 0) {
        m_model->takeItem(m_curListParent, m_curListRow);
        m_ownsCurList = true;
    }

    if (m_nextSiblingCount > 0) {
        if (m_subListCreated) {
            m_model->insertItem(m_listItem, m_listItem->childCount(), m_listItemSubList);
            m_ownsSubList = false;
        }

        m_model->moveItems(m_curList, m_listItemRow, m_nextSiblingCount, m_listItemSubList, m_listItemSubList->childCount());

        if (!m_ownsCurList && m_curList->childCount() == 0) {
            m_model->takeItem(m_curListParent, m_curListRow);
            m_ownsCurList = true;
        }

        m_model->updateListNumbers(m_listItemSubList, true);
    }

    m_model->updateListNumbers(m_targetList);
    m_model->requestFocus(m_listItem->child(0), m_cursorPosition);
}

bool DeIndentListItemCommand::isListOrdered(TreeItem *listItem)
{
    return listItem->itemAs<MD::ListItem>()->listType() == MD::ListItem::ListType::Ordered;
}