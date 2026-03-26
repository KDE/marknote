// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#ifndef EMOJIERPROXYMODEL_H
#define EMOJIERPROXYMODEL_H

#include "emojiermodel.h"
#include <QSortFilterProxyModel>
#include <qqmlintegration.h>

class EmojierProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit EmojierProxyModel(QObject *parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString &text);

Q_SIGNALS:
    void searchTextChanged();

private:
    QString m_searchText;
};

#endif // EMOJIERPROXYMODEL_H