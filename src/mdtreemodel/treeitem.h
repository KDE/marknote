// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TREEITEM_H
#define TREEITEM_H

#include "mdoptions/mdoptions.h"
#include <QObject>
#include <QVariant>
#include <md4qt/doc.h>

class TreeItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap data READ data CONSTANT)
    Q_PROPERTY(TreeItem *parent READ parent CONSTANT)
    Q_PROPERTY(QList<TreeItem *> children READ children CONSTANT)

public:
    explicit TreeItem(TreeItem *parent = nullptr, QObject *parentObject = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);
    void insertChild(int childRow, TreeItem *child);
    TreeItem *removeChild(int childRow);

    QList<TreeItem *> takeChildren(int startIndex, int count = -1);
    void insertChildren(int startIndex, const QList<TreeItem *> &children);
    void removeChildren(int startIndex, int count = -1);
    void moveChildren(int sourceRow, int count, TreeItem *destination, int destRow);

    TreeItem *child(int childRow);
    int childCount() const;

    int columnCount() const;
    TreeItem *parent();
    int row() const;

    QVariantMap data() const;
    MDOptions::ElementType type() const;

    QSharedPointer<MD::Item> item() const;

    template<typename T>
    QSharedPointer<T> itemAs() const
    {
        return m_item ? m_item.dynamicCast<T>() : QSharedPointer<T>();
    }

    QList<TreeItem *> children() const;
    bool isDescendantOf(TreeItem *other) const;

    static TreeItem *buildTree(const QSharedPointer<MD::Item> &item);
    static QSharedPointer<MD::Item> createMDItem(MDOptions::ElementType type, const QString &text = QString());
    static TreeItem *createTreeItem(MDOptions::ElementType type, const QString &text = QString());
    static QList<TreeItem *> fromMarkdown(const QString &markdown);

    void setUnparsedMarkdown(const QString &text);
    QString unparsedMarkdown() const;
    void clearUnparsedMarkdown();

private:
    QSharedPointer<MD::Item> m_item;
    QString m_unparsedMd;

    TreeItem *m_parent;
    QList<TreeItem *> m_children;
};

#endif // TREEITEM_H
