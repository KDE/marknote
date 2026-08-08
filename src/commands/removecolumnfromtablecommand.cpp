// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "removecolumnfromtablecommand.h"

using namespace Qt::StringLiterals;

RemoveColumnFromTableCommand::RemoveColumnFromTableCommand(TreeItem *block, int col, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_col(col)
{
    if (m_block && m_block->item() && m_block->item()->type() == MD::ItemType::Table) {
        auto table = m_block->itemAs<MD::Table>();
        for (const auto &rowItem : table->rows()) {
            if (m_col >= 0 && m_col < rowItem->cells().size()) {
                m_savedColData.append(rowItem->cells().at(m_col));
            } else {
                m_savedColData.append(nullptr);
            }
        }
    }

    QVariantMap dataMap = m_block->data();
    if (dataMap.contains(u"mdData"_s)) {
        QList<QVariantList> mdData = dataMap.value(u"mdData"_s).value<QList<QVariantList>>();
        for (const QVariantList &rowList : mdData) {
            if (m_col >= 0 && m_col < rowList.size()) {
                m_unparsedColData.append(rowList.at(m_col).toString());
            } else {
                m_unparsedColData.append(QString());
            }
        }
    }
}

RemoveColumnFromTableCommand::~RemoveColumnFromTableCommand()
{
}

void RemoveColumnFromTableCommand::undo()
{
    m_model->insertColInTable(m_block, m_col, m_savedColData, m_unparsedColData);
    m_model->requestFocusOnTable(m_block, 0, m_col, 0);
}

void RemoveColumnFromTableCommand::redo()
{
    m_model->removeColFromTable(m_block, m_col);
    m_model->requestFocusOnTable(m_block, 0, qMax(0, m_col - 1), 0);
}
