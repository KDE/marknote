// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidimageprovider.h"
#include "jsenginemermaidrenderer.h"
#include "mermaidcssflattener.h"

#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QSvgRenderer>

using namespace Qt::StringLiterals;

namespace {
QHash<QString, QString> &diagramCodeRegistry()
{
    static QHash<QString, QString> s_registry;
    return s_registry;
}

QMutex &registryMutex()
{
    static QMutex s_regMutex;
    return s_regMutex;
}
}

MermaidImageResponse::MermaidImageResponse(const QString &id, const QSize &requestedSize)
    : m_id(id)
    , m_requestedSize(requestedSize)
{
    startJob();
}

QQuickTextureFactory *MermaidImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString MermaidImageResponse::errorString() const
{
    return m_errorString;
}

void MermaidImageResponse::cancel()
{
    m_cancelled = true;
}

void MermaidImageResponse::startJob()
{
    if (m_cancelled) {
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QStringList parts = m_id.split(QLatin1Char('_'));
    if (parts.isEmpty()) {
        m_errorString = u"Invalid diagram image ID format"_s;
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QString hash = parts.at(0);
    const bool isDark = (parts.value(1) == u"1"_s);

    QString code = MermaidImageProvider::getDiagram(hash);
    if (code.isEmpty()) {
        m_errorString = u"Diagram source not found for hash"_s;
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    MermaidRenderRequest req;
    req.code = code;
    req.isDarkMode = isDark;

    JsEngineMermaidRenderer::instance()->renderAsync(req, [this, isDark](const MermaidRenderResult &result) {
        if (m_cancelled) {
            Q_EMIT finished();
            return;
        }

        if (!result.success) {
            m_errorString = result.errorString;
            Q_EMIT finished();
            return;
        }

        QString flatSvg = MermaidCssFlattener::flattenSvg(result.svg, {}, isDark);
        QByteArray svgData = flatSvg.toUtf8();
        QSvgRenderer renderer(svgData);
        if (!renderer.isValid()) {
            m_errorString = u"Failed to parse flattened SVG"_s;
            Q_EMIT finished();
            return;
        }

        QSize size = renderer.defaultSize();
        if (size.isEmpty()) {
            size = result.naturalSize.isValid() ? result.naturalSize : QSize(400, 300);
        }

        QImage img(size, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);

        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        renderer.render(&painter);
        painter.end();

        m_image = img;
        Q_EMIT finished();
    });
}

MermaidImageProvider::MermaidImageProvider()
    : QQuickAsyncImageProvider()
{
}

QQuickImageResponse *MermaidImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    return new MermaidImageResponse(id, requestedSize);
}

void MermaidImageProvider::registerDiagram(const QString &hash, const QString &code)
{
    QMutexLocker locker(&registryMutex());
    diagramCodeRegistry().insert(hash, code);
}

QString MermaidImageProvider::getDiagram(const QString &hash)
{
    QMutexLocker locker(&registryMutex());
    return diagramCodeRegistry().value(hash);
}
