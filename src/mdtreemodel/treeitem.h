// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>
#include <md4qt/doc.h>

class TreeItem
{
public:
    explicit TreeItem(TreeItem *parent = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);
    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariantMap data() const;
    TreeItem *parent();
    int row() const;

    static TreeItem *buildTree(const QSharedPointer<MD::Item> &item);
    static void traverseTree(TreeItem *item);

private:
    QSharedPointer<MD::Item> m_item;

    TreeItem *m_parent;
    QList<TreeItem *> m_children;
};

#endif // TREEITEM_H
