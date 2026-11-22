// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "mermaidrenderer.h"

#include <QThread>

class MermaidWorker;

class JsEngineMermaidRenderer : public MermaidRenderer
{
    Q_OBJECT

public:
    explicit JsEngineMermaidRenderer(QObject *parent = nullptr);
    ~JsEngineMermaidRenderer() override;

    void renderAsync(const MermaidRenderRequest &request, RenderCallback callback) override;
    MermaidRenderResult renderSync(const MermaidRenderRequest &request) override;

    static JsEngineMermaidRenderer *instance();

private:
    QThread m_workerThread;
    MermaidWorker *m_worker = nullptr;
};
