// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QString>
#include <memory>

struct MermaidRenderContext {
    QString fontFamily;
    qreal fontSize = 10.0;
    QHash<QString, QString> variables;
};

class MermaidImageResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    MermaidImageResponse(const QString &id, const QSize &requestedSize);
    ~MermaidImageResponse() override;

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;
    void cancel() override;

private:
    struct SharedState {
        QMutex mutex;
        QImage image;
        QString errorString;
        bool cancelled = false;
        bool destroyed = false;
    };

    void startJob();

    QString m_id;
    QSize m_requestedSize;
    std::shared_ptr<SharedState> m_state;
};

class MermaidImageProvider : public QQuickAsyncImageProvider
{
public:
    MermaidImageProvider();
    ~MermaidImageProvider() override = default;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

    static void registerDiagram(const QString &hash, const QString &code);
    static QString getDiagram(const QString &hash);
    static void registerRenderContext(const QString &diagramId, const MermaidRenderContext &context);
    static MermaidRenderContext getRenderContext(const QString &diagramId);
};
