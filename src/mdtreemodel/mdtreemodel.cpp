// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdtreemodel.h"
#include <QTimer>
#include <md4qt/parser.h>
using namespace Qt::StringLiterals;

MDTreeModel::MDTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(std::make_unique<TreeItem>())
{
}

QModelIndex MDTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : m_rootItem.get();

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);

    return QModelIndex();
}

QModelIndex MDTreeModel::indexFromItem(TreeItem *item) const
{
    if (!item || item == m_rootItem.get()) {
        return QModelIndex();
    }

    return createIndex(item->row(), 0, item);
}

QModelIndex MDTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem *>(child.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem.get() || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int MDTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : m_rootItem.get();

    return parentItem->childCount();
}

int MDTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant MDTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());

    if (role == Roles::BlockRole) {
        return QVariant::fromValue(item);
    }

    if (role == Roles::BlockTypeRole) {
        return item->data().value(u"type"_s);
    }

    return QVariant();
}

QHash<int, QByteArray> MDTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Roles::BlockRole] = "block";
    roles[Roles::BlockTypeRole] = "blockType";
    return roles;
}

void MDTreeModel::setDocument(const QSharedPointer<MD::Document> &document)
{
    beginResetModel();

    if (document) {
        TreeItem *newRoot = TreeItem::buildTree(document);
        m_rootItem.reset(newRoot);
    }

    endResetModel();
}

void MDTreeModel::markDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_EMIT dataChanged(topLeft, bottomRight);
}

void MDTreeModel::childRemoveBegin(TreeItem *parent, int row)
{
    QModelIndex parentIndex = indexFromItem(parent);
    beginRemoveRows(parentIndex, row, row);
}

void MDTreeModel::childRemoveBegin(TreeItem *parent, int rowStart, int rowEnd)
{
    QModelIndex parentIndex = indexFromItem(parent);
    beginRemoveRows(parentIndex, rowStart, rowEnd);
}

void MDTreeModel::childRemoveEnd()
{
    endRemoveRows();
}

void MDTreeModel::childAddBegin(TreeItem *parent, int row)
{
    QModelIndex parentIndex = indexFromItem(parent);
    beginInsertRows(parentIndex, row, row);
}

void MDTreeModel::childAddBegin(TreeItem *parent, int rowStart, int rowEnd)
{
    QModelIndex parentIndex = indexFromItem(parent);
    beginInsertRows(parentIndex, rowStart, rowEnd);
}

void MDTreeModel::childAddEnd()
{
    endInsertRows();
}

void MDTreeModel::childMoveBegin(TreeItem *parent, int rowStart, int rowEnd, TreeItem *newParent, int newRowStart)
{
    QModelIndex parentIndex = indexFromItem(parent);
    QModelIndex newParentIndex = indexFromItem(newParent);

    beginMoveRows(parentIndex, rowStart, rowEnd, newParentIndex, newRowStart);
}

void MDTreeModel::childMoveEnd()
{
    endMoveRows();
}

void MDTreeModel::childModified(TreeItem *parent, int rowStart, int rowEnd)
{
    QModelIndex parentIndex = indexFromItem(parent);
    Q_EMIT dataChanged(index(rowStart, 0, parentIndex), index(rowEnd, 0, parentIndex));
}

void MDTreeModel::setItemMD(TreeItem *block, const QString &md)
{
    block->setUnparsedMarkdown(md);
    if (block->parent()) {
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::setItemTableMD(TreeItem *block, int row, int col, const QString &md)
{
    if (!block || block->type() != MDOptions::ElementType::Table) {
        return;
    }
    block->setUnparsedMarkdownForTable(md, row, col);
    if (block->parent()) {
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::setTableCellMD(TreeItem *block, int row, int col, const QString &markdown)
{
    if (!block || block->type() != MDOptions::ElementType::Table) {
        return;
    }
    block->setTableCellMarkdown(row, col, markdown);
    if (block->parent()) {
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::setItemCode(TreeItem *block, const QString &code)
{
    if (block->type() != MDOptions::ElementType::Code) {
        return;
    }
    block->setCode(code);
    if (block->parent()) {
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::appendRowInTable(TreeItem *block)
{
    if (block) {
        block->appendRowInTable();
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::appendColInTable(TreeItem *block)
{
    if (block) {
        block->appendColInTable();
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::insertRowInTable(TreeItem *block, int row, QSharedPointer<MD::TableRow> rowToInsert, const QList<QString> &unparsedRowData)
{
    if (block) {
        block->insertRowInTable(row, rowToInsert, unparsedRowData);
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::insertColInTable(TreeItem *block, int col, const QList<QSharedPointer<MD::TableCell>> &colToInsert, const QList<QString> &unparsedColData)
{
    if (block) {
        block->insertColInTable(col, colToInsert, unparsedColData);
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::removeRowFromTable(TreeItem *block, int row)
{
    if (block) {
        block->removeRowFromTable(row);
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::removeColFromTable(TreeItem *block, int col)
{
    if (block) {
        block->removeColFromTable(col);
        childModified(block->parent(), block->row(), block->row());
    }
}

void MDTreeModel::insertItem(TreeItem *parent, int row, TreeItem *child)
{
    if (!parent || !child || row < 0 || row > parent->childCount()) {
        return;
    }

    QModelIndex parentIndex = indexFromItem(parent);
    beginInsertRows(parentIndex, row, row);
    parent->insertChild(row, child);
    endInsertRows();
}

void MDTreeModel::insertItems(TreeItem *parent, int row, const QList<TreeItem *> &children)
{
    if (!parent || children.isEmpty() || row < 0 || row > parent->childCount()) {
        return;
    }

    QModelIndex parentIndex = indexFromItem(parent);
    beginInsertRows(parentIndex, row, row + children.size() - 1);
    parent->insertChildren(row, children);
    endInsertRows();
}

TreeItem *MDTreeModel::takeItem(TreeItem *parent, int row)
{
    if (!parent || row < 0 || row >= parent->childCount()) {
        return nullptr;
    }

    QModelIndex parentIndex = indexFromItem(parent);
    beginRemoveRows(parentIndex, row, row);
    TreeItem *child = parent->removeChild(row);
    endRemoveRows();
    return child;
}

QList<TreeItem *> MDTreeModel::takeItems(TreeItem *parent, int row, int count)
{
    if (!parent || row < 0 || count <= 0 || row + count > parent->childCount()) {
        return QList<TreeItem *>();
    }

    QModelIndex parentIndex = indexFromItem(parent);
    beginRemoveRows(parentIndex, row, row + count - 1);
    QList<TreeItem *> children = parent->takeChildren(row, count);
    endRemoveRows();
    return children;
}

void MDTreeModel::removeItem(TreeItem *parent, int row)
{
    delete takeItem(parent, row);
}

void MDTreeModel::removeItems(TreeItem *parent, int row, int count)
{
    qDeleteAll(takeItems(parent, row, count));
}

void MDTreeModel::moveItem(TreeItem *item, TreeItem *newParent, int targetRow)
{
    if (!item || !newParent || targetRow < 0 || targetRow > newParent->childCount()) {
        return;
    }

    TreeItem *currentParent = item->parent();
    if (!currentParent) {
        return;
    }

    int currentRow = item->row();
    if (currentParent == newParent && currentRow == targetRow) {
        return;
    }

    if (newParent == item || newParent->isDescendantOf(item)) {
        return;
    }

    auto takenItem = takeItem(currentParent, currentRow);

    if (currentParent == newParent && currentRow < targetRow) {
        targetRow--;
    }

    insertItem(newParent, targetRow, takenItem);
}

void MDTreeModel::moveItems(TreeItem *sourceParent, int sourceRow, int count, TreeItem *destinationParent, int destinationChild)
{
    if (!sourceParent || !destinationParent || count <= 0 || sourceRow < 0 || sourceRow + count > sourceParent->childCount() || destinationChild < 0
        || destinationChild > destinationParent->childCount()) {
        return;
    }

    if (sourceParent == destinationParent && destinationChild >= sourceRow && destinationChild <= sourceRow + count) {
        return;
    }

    bool descendant = false;
    for (int i = 0; i < count; ++i) {
        TreeItem *item = sourceParent->child(sourceRow + i);
        if (destinationParent == item || destinationParent->isDescendantOf(item)) {
            descendant = true;
            break;
        }
    }
    if (descendant) {
        return;
    }

    auto takenItems = takeItems(sourceParent, sourceRow, count);

    if (sourceParent == destinationParent && destinationChild > sourceRow) {
        destinationChild -= count;
    }

    insertItems(destinationParent, destinationChild, takenItems);
}

void MDTreeModel::updateListNumbers(TreeItem *list, bool reset)
{
    if (!list || list->childCount() == 0) {
        return;
    }

    TreeItem *firstChild = list->child(0);
    auto listItem = firstChild->itemAs<MD::ListItem>();

    if (!listItem || listItem->listType() != MD::ListItem::ListType::Ordered) {
        return;
    }

    int offset = (reset ? 1 : listItem->startNumber());
    bool changed = false;

    for (int i = 0; i < list->childCount(); ++i) {
        TreeItem *child = list->child(i);
        auto item = child->itemAs<MD::ListItem>();
        if (item) {
            if (item->startNumber() != offset) {
                item->setStartNumber(offset);
                changed = true;
            }
            offset++;
        }
    }

    if (changed) {
        childModified(list, 0, list->childCount() - 1);
    }
}

void MDTreeModel::requestFocus(TreeItem *block, int cursorPos)
{
    while (block) {
        const auto data = block->data();
        if (data.contains(u"md"_s) || data.contains(u"text"_s)) {
            break;
        }

        block = block->child(0);
    }

    if (!block) {
        return;
    }

    if (cursorPos == -1) {
        cursorPos = block->data()[u"md"_s].value<QString>().length();
    }

    setFocusedBlock(block, cursorPos);
    Q_EMIT focusRequested(block, cursorPos);
}

void MDTreeModel::setFocusedBlock(TreeItem *block, int cursorPos)
{
    m_focusedBlock = block;
    m_focusedBlockCursorPos = cursorPos;
    m_focusedTableRow = -1;
    m_focusedTableColumn = -1;
}

void MDTreeModel::setFocusedBlockToTable(TreeItem *block, int row, int column, int cursorPos)
{
    if (!block || block->type() != MDOptions::ElementType::Table) {
        return;
    }

    m_focusedBlock = block;
    m_focusedBlockCursorPos = cursorPos;
    m_focusedTableRow = row;
    m_focusedTableColumn = column;
}

void MDTreeModel::requestFocusOnTable(TreeItem *block, int row, int column, int cursorPos)
{
    if (!block || block->type() != MDOptions::ElementType::Table) {
        return;
    }

    if (cursorPos == -1) {
        const auto data = block->data();
        if (data.contains(u"mdData"_s)) {
            QList<QVariantList> mdData = data[u"mdData"_s].value<QList<QVariantList>>();
            if (row >= 0 && row < mdData.size() && column >= 0 && column < mdData[row].size()) {
                cursorPos = mdData[row][column].toString().length();
            }
        }
    }

    setFocusedBlockToTable(block, row, column, cursorPos);
    Q_EMIT focusRequestedOnTable(block, row, column, cursorPos);
}

TreeItem *MDTreeModel::focusedBlock() const
{
    return m_focusedBlock;
}

int MDTreeModel::focusedBlockCursorPos() const
{
    if (m_focusedBlock) {
        return m_focusedBlockCursorPos;
    }

    return 0;
}

int MDTreeModel::focusedTableRow() const
{
    if (m_focusedBlock && m_focusedBlock->type() == MDOptions::ElementType::Table) {
        return m_focusedTableRow;
    }
    return -1;
}

int MDTreeModel::focusedTableColumn() const
{
    if (m_focusedBlock && m_focusedBlock->type() == MDOptions::ElementType::Table) {
        return m_focusedTableColumn;
    }
    return -1;
}