// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "togglecheckboxcommand.h"

ToggleCheckboxCommand::ToggleCheckboxCommand(TreeItem *block, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_listItemNode(block)
    , m_model(model)
    , m_wasChecked(false)
    , m_isChecked(false)
    , m_isValid(false)
{
    if (!m_model || !m_listItemNode) {
        return;
    }

    if (m_listItemNode->type() != MDOptions::ElementType::ListItem) {
        m_listItemNode = m_listItemNode->parent();
    }

    if (!m_listItemNode || m_listItemNode->type() != MDOptions::ElementType::ListItem) {
        return;
    }

    auto mdListItem = m_listItemNode->itemAs<MD::ListItem>();
    if (!mdListItem || !mdListItem->isTaskList()) {
        return;
    }

    m_wasChecked = mdListItem->isChecked();
    m_isChecked = !m_wasChecked;
    m_isValid = true;
}

ToggleCheckboxCommand::~ToggleCheckboxCommand()
{
}

void ToggleCheckboxCommand::undo()
{
    if (!m_isValid || !m_listItemNode) {
        return;
    }

    auto mdListItem = m_listItemNode->itemAs<MD::ListItem>();
    if (mdListItem) {
        mdListItem->setChecked(m_wasChecked);
    }

    Q_EMIT m_listItemNode->dataChanged();

    TreeItem *listNode = m_listItemNode->parent();
    if (listNode) {
        m_model->childModified(listNode, m_listItemNode->row(), m_listItemNode->row());
    }
}

void ToggleCheckboxCommand::redo()
{
    if (!m_isValid || !m_listItemNode) {
        return;
    }

    auto mdListItem = m_listItemNode->itemAs<MD::ListItem>();
    if (mdListItem) {
        mdListItem->setChecked(m_isChecked);
    }

    Q_EMIT m_listItemNode->dataChanged();

    TreeItem *listNode = m_listItemNode->parent();
    if (listNode) {
        m_model->childModified(listNode, m_listItemNode->row(), m_listItemNode->row());
    }
}
