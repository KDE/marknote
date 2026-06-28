// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDTREEMODEL_H
#define MDTREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QtQmlIntegration/qqmlintegration.h>
#include <md4qt/doc.h>
#include <memory>

class MDTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        BlockDataRole = Qt::UserRole + 1,
        BlockTypeRole
    };

    explicit MDTreeModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setDocument(const QSharedPointer<MD::Document> &document);

    void dumpTree() const;

private:
    std::unique_ptr<TreeItem> m_rootItem;
};

#endif // MDTREEMODEL_H
