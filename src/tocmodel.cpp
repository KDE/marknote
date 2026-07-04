// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "tocmodel.h"
#include "mdoptions/mdoptions.h"
#include <QRegularExpression>

TocModel::TocModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TocModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.count();
}

QVariant TocModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.count()) {
        return {};
    }

    const auto &entry = m_entries.at(index.row());
    switch (role) {
    case Role::Title:
        return entry.title;
    case Role::Level:
        return entry.level;
    case Role::BlockIndex:
        return entry.blockIndex;
    }

    return {};
}

QHash<int, QByteArray> TocModel::roleNames() const
{
    return {
        {Role::Title, "title"},
        {Role::Level, "level"},
        {Role::BlockIndex, "blockIndex"},
    };
}

MDTreeModel *TocModel::treeModel() const
{
    return m_treeModel;
}

void TocModel::setTreeModel(MDTreeModel *treeModel)
{
    if (m_treeModel == treeModel) {
        return;
    }

    if (m_treeModel) {
        disconnect(m_treeModel, &QAbstractItemModel::modelReset, this, &TocModel::updateModel);
        disconnect(m_treeModel, &QAbstractItemModel::dataChanged, this, &TocModel::updateModel);
        disconnect(m_treeModel, &QAbstractItemModel::rowsInserted, this, &TocModel::updateModel);
        disconnect(m_treeModel, &QAbstractItemModel::rowsRemoved, this, &TocModel::updateModel);
    }

    m_treeModel = treeModel;

    if (m_treeModel) {
        connect(m_treeModel, &QAbstractItemModel::modelReset, this, &TocModel::updateModel);
        connect(m_treeModel, &QAbstractItemModel::dataChanged, this, &TocModel::updateModel);
        connect(m_treeModel, &QAbstractItemModel::rowsInserted, this, &TocModel::updateModel);
        connect(m_treeModel, &QAbstractItemModel::rowsRemoved, this, &TocModel::updateModel);
        updateModel();
    } else {
        beginResetModel();
        m_entries.clear();
        endResetModel();
    }

    Q_EMIT treeModelChanged();
}

void TocModel::updateModel()
{
    if (!m_treeModel) {
        beginResetModel();
        m_entries.clear();
        endResetModel();
        return;
    }

    QList<Entry> newEntries;
    int rows = m_treeModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex idx = m_treeModel->index(i, 0);
        int type = m_treeModel->data(idx, MDTreeModel::BlockTypeRole).toInt();

        if (type == MDOptions::ElementType::Heading) {
            QVariantMap blockData = m_treeModel->data(idx, MDTreeModel::BlockDataRole).toMap();
            int level = blockData[QStringLiteral("level")].toInt();
            QString html = blockData[QStringLiteral("html")].toString();

            // Strip HTML tags
            QString title = html;
            title.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
            title = title.trimmed();

            newEntries.append({title, level, i});
        }
    }

    bool changed = (m_entries.count() != newEntries.count());
    if (!changed) {
        for (int i = 0; i < m_entries.count(); ++i) {
            if (m_entries[i].title != newEntries[i].title || m_entries[i].level != newEntries[i].level || m_entries[i].blockIndex != newEntries[i].blockIndex) {
                changed = true;
                break;
            }
        }
    }

    if (changed) {
        beginResetModel();
        m_entries = newEntries;
        endResetModel();
    }
}

int TocModel::headingIndexAtBlock(int blockIndex) const
{
    for (int i = rowCount() - 1; i >= 0; --i) {
        int headingPos = index(i, 0).data(Role::BlockIndex).toInt();

        if (headingPos <= blockIndex) {
            return i;
        }
    }

    return 0;
}

#include "moc_tocmodel.cpp"
