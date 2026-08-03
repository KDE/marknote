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