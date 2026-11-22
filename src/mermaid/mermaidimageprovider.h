// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>

class MermaidImageResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    MermaidImageResponse(const QString &id, const QSize &requestedSize);

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;
    void cancel() override;

private:
    void startJob();

    QString m_id;
    QSize m_requestedSize;
    QImage m_image;
    QString m_errorString;
    bool m_cancelled = false;
};

class MermaidImageProvider : public QQuickAsyncImageProvider
{
public:
    MermaidImageProvider();
    ~MermaidImageProvider() override = default;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

    static void registerDiagram(const QString &hash, const QString &code);
    static QString getDiagram(const QString &hash);
};
