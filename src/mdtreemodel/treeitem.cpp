// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treeitem.h"
#include "mddatagenerator.h"
#include <md4qt/html.h>
#include <md4qt/parser.h>
using namespace Qt::StringLiterals;

TreeItem::TreeItem(TreeItem *parent, QObject *parentObject)
    : QObject(parent ? parent : parentObject)
    , m_parent(parent)
{
}

TreeItem::~TreeItem()
{
}

void TreeItem::appendChild(TreeItem *child)
{
    m_children.append(child);
    child->m_parent = this;
    child->setParent(this);
}

void TreeItem::insertChild(int childRow, TreeItem *child)
{
    m_children.insert(childRow, child);
    child->m_parent = this;
    child->setParent(this);
}

TreeItem *TreeItem::removeChild(int childRow)
{
    if (childRow < 0 || childRow >= m_children.size())
        return nullptr;

    TreeItem *child = m_children.takeAt(childRow);
    child->m_parent = nullptr;
    return child;
}

QList<TreeItem *> TreeItem::takeChildren(int startIndex, int count)
{
    QList<TreeItem *> result;
    if (startIndex < 0 || startIndex >= m_children.size()) {
        return result;
    }

    if (count == -1 || startIndex + count > m_children.size()) {
        count = m_children.size() - startIndex;
    }

    for (int i = 0; i < count; ++i) {
        TreeItem *child = m_children.takeAt(startIndex);
        child->m_parent = nullptr;
        result.append(child);
    }

    return result;
}

void TreeItem::insertChildren(int startIndex, const QList<TreeItem *> &children)
{
    if (startIndex < 0) {
        startIndex = 0;
    }
    if (startIndex > m_children.size()) {
        startIndex = m_children.size();
    }

    int i = 0;
    for (TreeItem *child : children) {
        m_children.insert(startIndex + i, child);
        child->m_parent = this;
        child->setParent(this);
        i++;
    }
}

void TreeItem::removeChildren(int startIndex, int count)
{
    takeChildren(startIndex, count);
}

void TreeItem::moveChildren(int sourceRow, int count, TreeItem *destination, int destRow)
{
    if (!destination || count <= 0) {
        return;
    }

    if (this == destination && destRow >= sourceRow && destRow <= sourceRow + count) {
        return;
    }

    QList<TreeItem *> itemsToMove = takeChildren(sourceRow, count);
    if (itemsToMove.isEmpty()) {
        return;
    }

    if (this == destination && destRow > sourceRow) {
        destRow -= itemsToMove.size();
    }

    destination->insertChildren(destRow, itemsToMove);
}

QList<TreeItem *> TreeItem::fromMarkdown(const QString &markdown)
{
    if (markdown.isEmpty()) {
        return {createTreeItem(MDOptions::ElementType::Paragraph)};
    }

    QTextStream stream{markdown.toUtf8()};

    MD::Parser parser;
    auto doc = parser.parse(stream, u""_s, u""_s);

    QList<TreeItem *> items;
    for (const auto &item : doc->items()) {
        if (item->type() == MD::ItemType::Anchor) {
            continue;
        }

        items.append(buildTree(item));
    }

    return items;
}

TreeItem *TreeItem::child(int childRow)
{
    if (childRow < 0 || childRow >= m_children.size())
        return nullptr;
    return m_children.at(childRow);
}

int TreeItem::childCount() const
{
    return m_children.count();
}

int TreeItem::columnCount() const
{
    // we don't need more than 1 column
    return 1;
}

QVariantMap TreeItem::data() const
{
    if (!m_item) {
        return QVariantMap();
    }

    QVariantMap map;

    switch (m_item->type()) {
    case MD::ItemType::Heading:
        map = MDDataGenerator::fromHeading(m_item);
        break;
    case MD::ItemType::Paragraph:
        map = MDDataGenerator::fromParagraph(m_item);
        break;
    case MD::ItemType::Blockquote:
        map = MDDataGenerator::fromBlockquote(m_item);
        break;
    case MD::ItemType::ListItem:
        map = MDDataGenerator::fromListItem(m_item);
        break;
    case MD::ItemType::List:
        map = MDDataGenerator::fromList(m_item);
        break;
    case MD::ItemType::Code:
        map = MDDataGenerator::fromCodeBlock(m_item);
        break;
    case MD::ItemType::Table:
        map = MDDataGenerator::fromTable(m_item);
        break;
    case MD::ItemType::Footnote:
        map = MDDataGenerator::fromFootnote(m_item);
        break;
    case MD::ItemType::Document:
        break;
    case MD::ItemType::PageBreak:
        map = MDDataGenerator::fromPageBreak(m_item);
        break;
    case MD::ItemType::Anchor:
        map = MDDataGenerator::fromAnchor(m_item);
        break;
    case MD::ItemType::HorizontalLine:
        map = MDDataGenerator::fromHorizontalLine(m_item);
        break;
    default:
        break;
    }

    if (!m_unparsedMd.isNull()) {
        map[u"md"_s] = m_unparsedMd;
    }

    if (m_item->type() == MD::ItemType::Table && m_unparsedTableMd) {
        if (map.contains(u"mdData"_s)) {
            QList<QVariantList> mdData = map[u"mdData"_s].value<QList<QVariantList>>();

            for (int r = 0; r < m_unparsedTableMd->size() && r < mdData.size(); ++r) {
                for (int c = 0; c < (*m_unparsedTableMd)[r].size() && c < mdData[r].size(); ++c) {
                    const QString &unparsedText = (*m_unparsedTableMd)[r][c];
                    if (!unparsedText.isNull()) {
                        mdData[r][c] = unparsedText;
                    }
                }
            }

            map[u"mdData"_s] = QVariant::fromValue(mdData);
        }
    }

    return map;
}

MDOptions::ElementType TreeItem::type() const
{
    return data()[u"type"_s].value<MDOptions::ElementType>();
}

TreeItem *TreeItem::parent()
{
    return m_parent;
}

int TreeItem::row() const
{
    if (m_parent)
        return m_parent->m_children.indexOf(this);

    return 0;
}

QSharedPointer<MD::Item> TreeItem::item() const
{
    return m_item;
}

QList<TreeItem *> TreeItem::children() const
{
    return m_children;
}

bool TreeItem::isDescendantOf(TreeItem *other) const
{
    TreeItem *p = m_parent;
    while (p) {
        if (p == other)
            return true;
        p = p->m_parent;
    }
    return false;
}

TreeItem *TreeItem::buildTree(const QSharedPointer<MD::Item> &item)
{
    TreeItem *treeItem = new TreeItem();
    treeItem->m_item = item;

    // items that are not "blocks" don't have any children, so we can return early
    auto block = item.dynamicCast<MD::Block>();

    if (!block) {
        return treeItem;
    }

    for (auto it = block->items().cbegin(); it != block->items().cend(); ++it) {
        TreeItem *child = buildTree(*it);
        treeItem->appendChild(child);
    }

    if (treeItem->childCount() == 0) {
        treeItem->appendChild(createTreeItem(MDOptions::ElementType::Paragraph));
    }

    return treeItem;
}

QSharedPointer<MD::Item> TreeItem::createMDItem(MDOptions::ElementType type, const QString &text)
{
    switch (type) {
    case MDOptions::ElementType::Heading: {
        auto heading = QSharedPointer<MD::Heading>::create();
        heading->setText(createMDItem(MDOptions::ElementType::Paragraph, text).dynamicCast<MD::Paragraph>());
        return heading;
    }
    case MDOptions::ElementType::Paragraph: {
        auto paragraph = QSharedPointer<MD::Paragraph>::create();
        paragraph->appendItem(createMDItem(MDOptions::ElementType::Text, text));
        return paragraph;
    }
    case MDOptions::ElementType::Text: {
        auto textItem = QSharedPointer<MD::Text>::create();
        textItem->setText(text);
        return textItem;
    }
    case MDOptions::ElementType::Blockquote: {
        auto blockquote = QSharedPointer<MD::Blockquote>::create();
        blockquote->appendItem(createMDItem(MDOptions::ElementType::Paragraph, text));
        return blockquote;
    }
    case MDOptions::ElementType::ListItem: {
        auto listItem = QSharedPointer<MD::ListItem>::create();
        listItem->appendItem(createMDItem(MDOptions::ElementType::Paragraph, text));
        return listItem;
    }
    case MDOptions::ElementType::List: {
        auto list = QSharedPointer<MD::List>::create();
        list->appendItem(createMDItem(MDOptions::ElementType::ListItem, text));
        return list;
    }
    case MDOptions::ElementType::Code: {
        return QSharedPointer<MD::Code>::create(text, true, false);
    }
    case MDOptions::ElementType::Table: {
        return QSharedPointer<MD::Table>::create();
    }
    case MDOptions::ElementType::Footnote: {
        return QSharedPointer<MD::Footnote>::create();
    }
    case MDOptions::ElementType::HorizontalLine: {
        return QSharedPointer<MD::HorizontalLine>::create();
    }
    default: {
        return createMDItem(MDOptions::ElementType::Paragraph, text);
    }
    }
}

TreeItem *TreeItem::createTreeItem(MDOptions::ElementType type, const QString &text)
{
    return buildTree(createMDItem(type, text));
}

void TreeItem::setUnparsedMarkdown(const QString &text)
{
    m_unparsedMd = text;
}

QString TreeItem::unparsedMarkdown() const
{
    return m_unparsedMd;
}

void TreeItem::clearUnparsedMarkdown()
{
    m_unparsedMd.clear();
}

void TreeItem::setUnparsedMarkdownForTable(const QString &text, int row, int col)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto table = m_item.dynamicCast<MD::Table>();
    if (!table) {
        return;
    }

    if (!m_unparsedTableMd) {
        int rowCount = table->rows().size();
        int colCount = table->columnsCount();
        m_unparsedTableMd = std::make_unique<QList<QList<QString>>>();
        for (int r = 0; r < rowCount; ++r) {
            QList<QString> rowData;
            for (int c = 0; c < colCount; ++c) {
                rowData.append(QString());
            }
            m_unparsedTableMd->append(rowData);
        }
    }

    if (row >= 0 && row < m_unparsedTableMd->size() && col >= 0 && col < (*m_unparsedTableMd)[row].size()) {
        (*m_unparsedTableMd)[row][col] = text;
    }
}

void TreeItem::clearUnparsedTableMarkdown()
{
    m_unparsedTableMd.reset();
}

void TreeItem::setCode(const QString &text)
{
    if (m_item->type() != MD::ItemType::Code) {
        return;
    }

    auto codeItem = m_item.dynamicCast<MD::Code>();
    codeItem->setText(text);
}

void TreeItem::setTableCellMarkdown(int row, int col, const QString &markdown)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto tableItem = m_item.dynamicCast<MD::Table>();

    if (row < 0 || row >= tableItem->rows().size()) {
        return;
    }

    auto tableRow = tableItem->rows()[row];
    if (col < 0 || col >= tableRow->cells().size()) {
        return;
    }

    QString markdownInTable{QString(u"|%1|\n|-|"_s).arg(markdown)};
    const auto output = fromMarkdown(markdownInTable);

    if (output.empty()) {
        return;
    }

    const auto parsedTable = output[0]->item().dynamicCast<MD::Table>();
    if (parsedTable && !parsedTable->rows().empty() && !parsedTable->rows()[0]->cells().empty()) {
        auto parsedCell = parsedTable->rows()[0]->cells()[0].dynamicCast<MD::Block>();
        auto targetCell = tableRow->cells()[col].dynamicCast<MD::Block>();

        for (int i = targetCell->items().size() - 1; i >= 0; --i) {
            targetCell->removeItemAt(i);
        }

        for (const auto &inlineItem : parsedCell->items()) {
            targetCell->appendItem(inlineItem);
        }
    }
}

void TreeItem::appendRowInTable()
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto tableItem = m_item.dynamicCast<MD::Table>();
    auto newRow = QSharedPointer<MD::TableRow>::create();

    int colCount = tableItem->columnsCount();
    if (colCount == 0 && tableItem->rows().size() > 0) {
        colCount = tableItem->rows()[0]->cells().size();
    }
    if (colCount == 0) {
        colCount = 1;
        tableItem->setColumnAlignment(0, MD::Table::AlignLeft);
    }

    for (int i = 0; i < colCount; ++i) {
        auto newCell = QSharedPointer<MD::TableCell>::create();
        auto newText = QSharedPointer<MD::Text>::create();
        newText->setText(QString());
        newCell->appendItem(newText);
        newRow->appendCell(newCell);
    }

    tableItem->appendRow(newRow);

    if (m_unparsedTableMd) {
        QList<QString> newMdRow;
        for (int i = 0; i < colCount; ++i) {
            newMdRow.append(QString());
        }
        m_unparsedTableMd->append(newMdRow);
    }
}

void TreeItem::appendColInTable()
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto tableItem = m_item.dynamicCast<MD::Table>();

    tableItem->setColumnAlignment(tableItem->columnsCount(), MD::Table::AlignLeft);

    for (const auto &row : tableItem->rows()) {
        auto newCell = QSharedPointer<MD::TableCell>::create();
        auto newText = QSharedPointer<MD::Text>::create();
        newText->setText(QString());
        newCell->appendItem(newText);
        row->appendCell(newCell);
    }

    if (m_unparsedTableMd) {
        for (auto &mdRow : *m_unparsedTableMd) {
            mdRow.append(QString());
        }
    }
}

void TreeItem::insertRowInTable(int rowToInsert, QSharedPointer<MD::TableRow> rowData, const QList<QString> &unparsedMdRow)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto oldTable = m_item.dynamicCast<MD::Table>();
    if (rowToInsert < 0 || rowToInsert > oldTable->rows().size()) {
        return;
    }

    int colCount = oldTable->columnsCount();
    if (colCount == 0 && oldTable->rows().size() > 0) {
        colCount = oldTable->rows()[0]->cells().size();
    }
    if (colCount == 0) {
        colCount = 1;
    }

    auto newTable = QSharedPointer<MD::Table>::create();
    for (int c = 0; c < oldTable->columnsCount(); ++c) {
        newTable->setColumnAlignment(c, oldTable->columnAlignment(c));
    }

    for (int r = 0; r <= oldTable->rows().size(); ++r) {
        if (r == rowToInsert) {
            auto newRow = QSharedPointer<MD::TableRow>::create();
            if (rowData) {
                for (int c = 0; c < rowData->cells().size(); ++c) {
                    auto oldCell = rowData->cells()[c];
                    auto newCell = QSharedPointer<MD::TableCell>::create();
                    for (const auto &item : oldCell->items()) {
                        newCell->appendItem(item);
                    }
                    newRow->appendCell(newCell);
                }
            } else {
                for (int c = 0; c < colCount; ++c) {
                    auto newCell = QSharedPointer<MD::TableCell>::create();
                    auto newText = QSharedPointer<MD::Text>::create();
                    newText->setText(QString());
                    newCell->appendItem(newText);
                    newRow->appendCell(newCell);
                }
            }
            newTable->appendRow(newRow);
        }

        if (r < oldTable->rows().size()) {
            auto oldRow = oldTable->rows()[r];
            auto newRow = QSharedPointer<MD::TableRow>::create();

            for (int c = 0; c < oldRow->cells().size(); ++c) {
                auto oldCell = oldRow->cells()[c];
                auto newCell = QSharedPointer<MD::TableCell>::create();

                for (const auto &item : oldCell->items()) {
                    newCell->appendItem(item);
                }
                newRow->appendCell(newCell);
            }
            newTable->appendRow(newRow);
        }
    }

    m_item = newTable;

    if (m_unparsedTableMd) {
        if (rowToInsert >= 0 && rowToInsert <= m_unparsedTableMd->size()) {
            QList<QString> mdRow;
            if (!unparsedMdRow.isEmpty()) {
                mdRow = unparsedMdRow;
            } else {
                for (int c = 0; c < colCount; ++c) {
                    mdRow.append(QString());
                }
            }
            m_unparsedTableMd->insert(rowToInsert, mdRow);
        }
    }
}

void TreeItem::insertColInTable(int colToInsert, const QList<QSharedPointer<MD::TableCell>> &colData, const QList<QString> &unparsedMdCol)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto oldTable = m_item.dynamicCast<MD::Table>();
    int colCount = oldTable->columnsCount();
    if (colCount == 0 && oldTable->rows().size() > 0) {
        colCount = oldTable->rows()[0]->cells().size();
    }

    if (colToInsert < 0 || colToInsert > colCount) {
        return;
    }

    auto newTable = QSharedPointer<MD::Table>::create();
    for (int c = 0, newC = 0; c <= oldTable->columnsCount(); ++c) {
        if (c == colToInsert) {
            newTable->setColumnAlignment(newC++, MD::Table::AlignLeft);
        }
        if (c < oldTable->columnsCount()) {
            newTable->setColumnAlignment(newC++, oldTable->columnAlignment(c));
        }
    }

    for (int r = 0; r < oldTable->rows().size(); ++r) {
        auto oldRow = oldTable->rows()[r];
        auto newRow = QSharedPointer<MD::TableRow>::create();

        for (int c = 0; c <= oldRow->cells().size(); ++c) {
            if (c == colToInsert) {
                auto newCell = QSharedPointer<MD::TableCell>::create();
                if (r < colData.size() && colData[r]) {
                    for (const auto &item : colData[r]->items()) {
                        newCell->appendItem(item);
                    }
                } else {
                    auto newText = QSharedPointer<MD::Text>::create();
                    newText->setText(QString());
                    newCell->appendItem(newText);
                }
                newRow->appendCell(newCell);
            }

            if (c < oldRow->cells().size()) {
                auto oldCell = oldRow->cells()[c];
                auto newCell = QSharedPointer<MD::TableCell>::create();

                for (const auto &item : oldCell->items()) {
                    newCell->appendItem(item);
                }
                newRow->appendCell(newCell);
            }
        }
        newTable->appendRow(newRow);
    }

    m_item = newTable;

    if (m_unparsedTableMd) {
        for (int r = 0; r < m_unparsedTableMd->size(); ++r) {
            if (colToInsert >= 0 && colToInsert <= (*m_unparsedTableMd)[r].size()) {
                QString cellMd = (r < unparsedMdCol.size()) ? unparsedMdCol[r] : QString();
                (*m_unparsedTableMd)[r].insert(colToInsert, cellMd);
            }
        }
    }
}

void TreeItem::removeRowFromTable(int rowToRemove)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto oldTable = m_item.dynamicCast<MD::Table>();
    if (rowToRemove < 0 || rowToRemove >= oldTable->rows().size()) {
        return;
    }

    auto newTable = QSharedPointer<MD::Table>::create();
    for (int c = 0; c < oldTable->columnsCount(); ++c) {
        newTable->setColumnAlignment(c, oldTable->columnAlignment(c));
    }

    for (int r = 0; r < oldTable->rows().size(); ++r) {
        if (r == rowToRemove) {
            continue;
        }

        auto oldRow = oldTable->rows()[r];
        auto newRow = QSharedPointer<MD::TableRow>::create();

        for (int c = 0; c < oldRow->cells().size(); ++c) {
            auto oldCell = oldRow->cells()[c];
            auto newCell = QSharedPointer<MD::TableCell>::create();

            for (const auto &item : oldCell->items()) {
                newCell->appendItem(item);
            }
            newRow->appendCell(newCell);
        }
        newTable->appendRow(newRow);
    }

    m_item = newTable;

    if (m_unparsedTableMd) {
        if (rowToRemove >= 0 && rowToRemove < m_unparsedTableMd->size()) {
            m_unparsedTableMd->removeAt(rowToRemove);
        }
    }
}

void TreeItem::removeColFromTable(int colToRemove)
{
    if (m_item->type() != MD::ItemType::Table) {
        return;
    }

    auto oldTable = m_item.dynamicCast<MD::Table>();
    int colCount = oldTable->columnsCount();
    if (colCount == 0 && oldTable->rows().size() > 0) {
        colCount = oldTable->rows()[0]->cells().size();
    }

    if (colToRemove < 0 || colToRemove >= colCount) {
        return;
    }

    auto newTable = QSharedPointer<MD::Table>::create();
    for (int c = 0, newC = 0; c < oldTable->columnsCount(); ++c) {
        if (c == colToRemove)
            continue;
        newTable->setColumnAlignment(newC++, oldTable->columnAlignment(c));
    }

    for (int r = 0; r < oldTable->rows().size(); ++r) {
        auto oldRow = oldTable->rows()[r];
        auto newRow = QSharedPointer<MD::TableRow>::create();

        for (int c = 0; c < oldRow->cells().size(); ++c) {
            if (c == colToRemove) {
                continue;
            }

            auto oldCell = oldRow->cells()[c];
            auto newCell = QSharedPointer<MD::TableCell>::create();

            for (const auto &item : oldCell->items()) {
                newCell->appendItem(item);
            }
            newRow->appendCell(newCell);
        }
        newTable->appendRow(newRow);
    }

    m_item = newTable;

    if (m_unparsedTableMd) {
        for (auto &mdRow : *m_unparsedTableMd) {
            if (colToRemove >= 0 && colToRemove < mdRow.size()) {
                mdRow.removeAt(colToRemove);
            }
        }
    }
}