// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDTREEMODEL_H
#define MDTREEMODEL_H

#include "mdoptions/mdoptions.h"
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
        BlockRole = Qt::UserRole + 1,
        BlockTypeRole,
    };

    explicit MDTreeModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex indexFromItem(TreeItem *item) const;

    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void markDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void setDocument(const QSharedPointer<MD::Document> &document);

public:
    void childAddBegin(TreeItem *parent, int row);
    void childAddBegin(TreeItem *parent, int rowStart, int rowEnd);
    void childAddEnd();
    void childRemoveBegin(TreeItem *parent, int row);
    void childRemoveBegin(TreeItem *parent, int rowStart, int rowEnd);
    void childRemoveEnd();
    void childMoveBegin(TreeItem *parent, int rowStart, int rowEnd, TreeItem *newParent, int newRowStart);
    void childMoveEnd();
    void childModified(TreeItem *parent, int rowStart, int rowEnd);
    void setItemMD(TreeItem *block, const QString &md);

    void insertItem(TreeItem *parent, int row, TreeItem *child);
    void insertItems(TreeItem *parent, int row, const QList<TreeItem *> &children);
    void removeItem(TreeItem *parent, int row);
    void removeItems(TreeItem *parent, int row, int count);
    void moveItem(TreeItem *item, TreeItem *newParent, int targetRow);
    void moveItems(TreeItem *sourceParent, int sourceRow, int count, TreeItem *destinationParent, int destinationChild);
    TreeItem *takeItem(TreeItem *parent, int row);
    QList<TreeItem *> takeItems(TreeItem *parent, int row, int count);
    void updateListNumbers(TreeItem *list, bool reset = false);

    Q_INVOKABLE void requestFocus(TreeItem *block, int cursorPos = -1);
    Q_INVOKABLE void setFocusedBlock(TreeItem *block, int cursorPos);
    Q_INVOKABLE TreeItem *focusedBlock() const;
    Q_INVOKABLE int focusedBlockCursorPos() const;

Q_SIGNALS:
    void focusRequested(TreeItem *block, int cursorPosition);

private:
    std::unique_ptr<TreeItem> m_rootItem;
    TreeItem *m_pendingFocusBlock = nullptr;
    int m_pendingCursorPos = -1;
};

#endif // MDTREEMODEL_H
