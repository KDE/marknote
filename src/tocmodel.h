// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "mdtreemodel/mdtreemodel.h"
#include <QAbstractListModel>
#include <QPointer>

class TocModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(MDTreeModel *treeModel READ treeModel WRITE setTreeModel NOTIFY treeModelChanged REQUIRED)

public:
    enum Role {
        Title = Qt::UserRole + 1,
        Level,
        BlockIndex,
    };
    Q_ENUM(Role)

    explicit TocModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    MDTreeModel *treeModel() const;
    void setTreeModel(MDTreeModel *treeModel);

    Q_INVOKABLE int headingIndexAtBlock(int blockIndex) const;

Q_SIGNALS:
    void treeModelChanged();

private:
    void updateModel();

    QPointer<MDTreeModel> m_treeModel;
    struct Entry {
        QString title;
        int level;
        int blockIndex;
    };
    QList<Entry> m_entries;
};
