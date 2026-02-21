// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <KColorSchemeManager>
#include <QAbstractItemModel>

#include "colorschemer.h"

ColorSchemer::ColorSchemer(QObject *parent)
    : QObject(parent)
#if KCOLORSCHEME_VERSION < QT_VERSION_CHECK(6, 6, 0)
    , c(new KColorSchemeManager(this))
#else
    , c(KColorSchemeManager::instance())
#endif
{
}

ColorSchemer::~ColorSchemer()
{
}

QAbstractItemModel *ColorSchemer::model() const
{
    return c->model();
}

void ColorSchemer::apply(int idx)
{
    if (idx < 0) {
        return;
    }

    auto model = c->model();
    if (!model) {
        return;
    }

    QModelIndex sourceIndex = model->index(idx, 0);
    if (sourceIndex.isValid()) {
        c->activateScheme(sourceIndex);
    }
}

void ColorSchemer::apply(const QString &name)
{
    c->activateScheme(c->indexForScheme(name));
}

int ColorSchemer::indexForScheme(const QString &name) const
{
    auto index = c->indexForScheme(name).row();
    if (index == -1) {
        index = 0;
    }
    return index;
}

QString ColorSchemer::nameForIndex(int index) const
{
    return c->model()->data(c->model()->index(index, 0), Qt::DisplayRole).toString();
}

#include "moc_colorschemer.cpp"
