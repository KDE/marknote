// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "transformtochecklist.h"

using namespace Qt::StringLiterals;

TransformToChecklistCommand::TransformToChecklistCommand(TreeItem *block, bool isChecked, const QString &text, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_isChecked(isChecked)
    , m_text(text)
{
    m_oldText = block->data().value(u"md"_s).toString();

    TreeItem *listItemNode = m_block->parent();
    auto mdListItem = listItemNode->itemAs<MD::ListItem>();
    m_wasTaskList = mdListItem->isTaskList();
    m_wasChecked = mdListItem->isChecked();
}

TransformToChecklistCommand::~TransformToChecklistCommand()
{
}

void TransformToChecklistCommand::undo()
{
    TreeItem *listItemNode = m_block->parent();
    auto mdListItem = listItemNode->itemAs<MD::ListItem>();

    mdListItem->setTaskList(m_wasTaskList);
    if (m_wasTaskList) {
        mdListItem->setChecked(m_wasChecked);
    }

    m_model->setItemMD(m_block, m_oldText);
    m_model->childModified(listItemNode->parent(), listItemNode->row(), listItemNode->row());

    int offset = m_oldText.length() - m_text.length();
    if (offset < 0)
        offset = 0;
    m_model->requestFocus(m_block, offset);
}

void TransformToChecklistCommand::redo()
{
    TreeItem *listItemNode = m_block->parent();
    auto mdListItem = listItemNode->itemAs<MD::ListItem>();

    mdListItem->setTaskList(true);
    mdListItem->setChecked(m_isChecked);

    m_model->setItemMD(m_block, m_text);
    m_model->childModified(listItemNode->parent(), listItemNode->row(), listItemNode->row());

    m_model->requestFocus(m_block, 0);
}
