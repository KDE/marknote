// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDTREEMODEL_H
#define MDTREEMODEL_H

#include <QAbstractItemModel>
#include <QtQmlIntegration/qqmlintegration.h>
#include <md4qt/doc.h>

class MDTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MDTreeModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    MD::Document *m_document;
};

#endif // MDTREEMODEL_H
