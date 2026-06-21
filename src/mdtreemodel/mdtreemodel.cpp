// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdtreemodel.h"

MDTreeModel::MDTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex MDTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
}

QModelIndex MDTreeModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int MDTreeModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

int MDTreeModel::columnCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant MDTreeModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QHash<int, QByteArray> MDTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Roles::DataRole] = "data";
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

void MDTreeModel::dumpTree() const
{
    if (m_rootItem) {
        qDebug() << "Dumping tree structure:";
        std::function<void(const TreeItem *, int)> dump = [&](const TreeItem *item, int depth) {
            QString indent(depth * 2, ' ');
            qDebug() << indent << item->data();
            for (int i = 0; i < item->childCount(); ++i) {
                dump(item->child(i), depth + 1);
            }
        };
        dump(m_rootItem.get(), 0);
    } else {
        qDebug() << "Tree is empty.";
    }

}