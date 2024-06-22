// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "config.h"
#include <QFontDatabase>

Config::Config(QObject *parent)
    : QObject(parent)
{
}

QStringList Config::fontFamilies() const
{
    return QFontDatabase::families();
}
