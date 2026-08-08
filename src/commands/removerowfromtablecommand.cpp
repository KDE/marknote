// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "removerowfromtablecommand.h"

using namespace Qt::StringLiterals;

RemoveRowFromTableCommand::RemoveRowFromTableCommand(TreeItem *block, int row, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_block(block)
    , m_model(model)
    , m_row(row)
{
    if (m_block && m_block->item() && m_block->item()->type() == MD::ItemType::Table) {
        auto table = m_block->itemAs<MD::Table>();
        if (m_row >= 0 && m_row < table->rows().size()) {
            m_savedRowData = table->rows().at(m_row);
        }
    }

    QVariantMap dataMap = m_block->data();
    if (dataMap.contains(u"mdData"_s)) {
        QList<QVariantList> mdData = dataMap.value(u"mdData"_s).value<QList<QVariantList>>();
        if (m_row >= 0 && m_row < mdData.size()) {
            const QVariantList &rowList = mdData.at(m_row);
            for (const QVariant &cell : rowList) {
                m_unparsedRowData.append(cell.toString());
            }
        }
    }
}

RemoveRowFromTableCommand::~RemoveRowFromTableCommand()
{
}

void RemoveRowFromTableCommand::undo()
{
    m_model->insertRowInTable(m_block, m_row, m_savedRowData, m_unparsedRowData);
    m_model->requestFocusOnTable(m_block, m_row, 0, 0);
}

void RemoveRowFromTableCommand::redo()
{
    m_model->removeRowFromTable(m_block, m_row);
    m_model->requestFocusOnTable(m_block, qMax(0, m_row - 1), 0, 0);
}
