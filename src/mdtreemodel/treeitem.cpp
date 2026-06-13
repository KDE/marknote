// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "treeitem.h"
#include "mddatagenerator.h"
#include <md4qt/html.h>
using namespace Qt::StringLiterals;

TreeItem::TreeItem(TreeItem *parent)
    : m_parent(parent)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_children);
}

void TreeItem::appendChild(TreeItem *child)
{
    m_children.append(child);
    child->m_parent = this;
}

TreeItem *TreeItem::child(int row)
{
    if (row < 0 || row >= m_children.size())
        return nullptr;
    return m_children.at(row);
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

    switch (m_item->type()) {
    case MD::ItemType::Heading:
        return MDDataGenerator::fromHeading(m_item);
    case MD::ItemType::Paragraph:
        return MDDataGenerator::fromParagraph(m_item);
    case MD::ItemType::Blockquote:
        return MDDataGenerator::fromBlockquote(m_item);
    case MD::ItemType::ListItem:
        return MDDataGenerator::fromListItem(m_item);
    case MD::ItemType::List:
        return MDDataGenerator::fromList(m_item);
    case MD::ItemType::Code:
        return MDDataGenerator::fromCodeBlock(m_item);
    case MD::ItemType::Table:
        return MDDataGenerator::fromTable(m_item);
    case MD::ItemType::Footnote:
        return MDDataGenerator::fromFootnote(m_item);
    case MD::ItemType::Document:
        return {};
    case MD::ItemType::PageBreak:
        return MDDataGenerator::fromPageBreak(m_item);
    case MD::ItemType::Anchor:
        return MDDataGenerator::fromAnchor(m_item);
    case MD::ItemType::HorizontalLine:
        return MDDataGenerator::fromHorizontalLine(m_item);
    default:
        break;
    }

    return QVariantMap();
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

TreeItem *TreeItem::buildTree(const QSharedPointer<MD::Item> &item)
{
    TreeItem *treeItem = new TreeItem();
    treeItem->m_item = item;

    // items that are not "blocks" don't have any children, so we can return early
    auto block = item.dynamicCast<MD::Block>();

    if (!block || block->type() == MD::ItemType::Paragraph) {
        return treeItem;
    }

    for (auto it = block->items().cbegin(); it != block->items().cend(); ++it) {
        TreeItem *child = buildTree(*it);
        treeItem->appendChild(child);
    }

    return treeItem;
}

namespace
{
int indent = 0;
};

void TreeItem::traverseTree(TreeItem *item)
{
    if (!item) {
        return;
    }

    indent++;
    qDebug() << std::string(indent * 2, ' ') << "Item Type:" << item->data().value(u"type"_s);

    for (int i = 0; i < item->childCount(); ++i) {
        traverseTree(item->child(i));
    }
    indent--;
}
