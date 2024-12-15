// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QtQml>
#include <marknotesettings.h>

class ConfigHelper : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ConfigHelper)
    QML_SINGLETON

    Q_PROPERTY(QStringList fontFamilies READ fontFamilies CONSTANT)

public:
    explicit ConfigHelper(QObject *parent = nullptr);

    QStringList fontFamilies() const;
};
