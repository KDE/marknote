// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#include "emojierproxymodel.h"

EmojierProxyModel::EmojierProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    QSortFilterProxyModel::setSourceModel(new EmojierModel(this));
    setFilterRole(EmojierModel::ShortcodeRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QString EmojierProxyModel::searchText() const
{
    return m_searchText;
}

void EmojierProxyModel::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        setFilterFixedString(m_searchText);
        Q_EMIT searchTextChanged();
    }
}
