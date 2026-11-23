// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QHash>
#include <QMap>
#include <QString>

class MermaidCssFlattener
{
public:
    static QHash<QString, QString> defaultVariables(bool isDark);
    static QString flattenSvg(const QString &rawSvg, const QHash<QString, QString> &customVariables, bool isDark);
    static QString normalizeTspans(const QString &svg);
};
