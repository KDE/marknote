// SPDX-FileCopyrightText: 2024 Gary Wang <opensource@blumia.net>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include "marknotesettings.h"

struct ForeignConfig {
    Q_GADGET
    QML_FOREIGN(MarknoteSettings)
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON
public:
    static MarknoteSettings *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(MarknoteSettings::self(), QQmlEngine::CppOwnership);
        return MarknoteSettings::self();
    }
};
