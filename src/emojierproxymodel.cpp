//
// Created by siddharth on 3/26/26.
//

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
