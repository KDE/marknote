// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdtreemodel.h"
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

    if (role == Roles::BlockDataRole) {
        return item->data();
    }

    if (role == Roles::BlockTypeRole) {
        return item->data()[u"type"_s];
    }

    return QVariant();
}

QHash<int, QByteArray> MDTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Roles::BlockDataRole] = "blockData";
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