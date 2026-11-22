// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

class MermaidHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit MermaidHelper(QObject *parent = nullptr);

    Q_INVOKABLE QString diagramId(const QString &code, bool isDark);
    Q_INVOKABLE QString getCacheKey(const QString &code, bool isDark, int widthBucket = 0, qreal dpr = 1.0);
    Q_INVOKABLE bool isMermaid(const QString &language, const QString &code);
};
