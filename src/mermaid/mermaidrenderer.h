// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>
#include <QSize>
#include <QString>
#include <functional>

struct MermaidRenderRequest {
    QString code;
    QString fontFamily;
    qreal fontSize = 10.0;
    qreal devicePixelRatio = 1.0;
    int targetWidth = 0;
    bool isDarkMode = false;
    QHash<QString, QString> colorOverrides;
};

struct MermaidRenderResult {
    QString svg;
    QSize naturalSize;
    QString errorString;
    bool success = false;
};

class MermaidRenderer : public QObject
{
    Q_OBJECT
public:
    explicit MermaidRenderer(QObject *parent = nullptr) : QObject(parent) {}
    ~MermaidRenderer() override = default;

    using RenderCallback = std::function<void(const MermaidRenderResult &result)>;

    virtual void renderAsync(const MermaidRenderRequest &request, RenderCallback callback) = 0;
    virtual MermaidRenderResult renderSync(const MermaidRenderRequest &request) = 0;
};

