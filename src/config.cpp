// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "config.h"
#include <QFontDatabase>

ConfigHelper::ConfigHelper(QObject *parent)
    : QObject(parent)
{
}

QStringList ConfigHelper::fontFamilies() const
{
    return QFontDatabase::families();
}

#include "moc_config.cpp"
