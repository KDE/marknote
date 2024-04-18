// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QtQml>
#include <marknotesettings.h>

class Config : public MarknoteSettings
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList fontFamilies READ fontFamilies CONSTANT)

public:
    explicit Config(QObject *parent = nullptr);

    QStringList fontFamilies() const;

    Q_INVOKABLE void reset();
};
