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
