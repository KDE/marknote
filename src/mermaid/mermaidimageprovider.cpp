// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidimageprovider.h"
#include "jsenginemermaidrenderer.h"
#include "mermaidcssflattener.h"
#include "mermaidsvgcache.h"
#include <marknotesettings.h>

#include <QFontDatabase>
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

QImage renderSvgToImage(const QString &flatSvg, const QSize &naturalSize, QString &errorString)
{
    QByteArray svgData = flatSvg.toUtf8();
    QSvgRenderer renderer(svgData);
    if (!renderer.isValid()) {
        errorString = u"Failed to parse flattened SVG"_s;
        return QImage();
    }

    QSize size = renderer.defaultSize();
    if (size.isEmpty()) {
        size = naturalSize.isValid() ? naturalSize : QSize(400, 300);
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer.render(&painter);
    painter.end();

    return img;
}
} // namespace

MermaidImageResponse::MermaidImageResponse(const QString &id, const QSize &requestedSize)
    : m_id(id)
    , m_requestedSize(requestedSize)
{
    startJob();
}

QQuickTextureFactory *MermaidImageResponse::textureFactory() const
{
    QMutexLocker locker(&m_mutex);
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString MermaidImageResponse::errorString() const
{
    QMutexLocker locker(&m_mutex);
    return m_errorString;
}

void MermaidImageResponse::cancel()
{
    m_cancelled.store(true, std::memory_order_relaxed);
}

void MermaidImageResponse::startJob()
{
    if (m_cancelled.load(std::memory_order_relaxed)) {
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QStringList parts = m_id.split(QLatin1Char('_'));
    if (parts.isEmpty()) {
        {
            QMutexLocker locker(&m_mutex);
            m_errorString = u"Invalid diagram image ID format"_s;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QString hash = parts.at(0);
    const bool isDark = (parts.value(1) == u"1"_s);

    // Fast Path: Check 50 MB SVG Cache
    if (auto cached = MermaidSvgCache::instance()->get(m_id)) {
        QString err;
        QImage img = renderSvgToImage(cached->flatSvg, cached->naturalSize, err);
        {
            QMutexLocker locker(&m_mutex);
            m_image = img;
            m_errorString = err;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    QString code = MermaidImageProvider::getDiagram(hash);
    if (code.isEmpty()) {
        {
            QMutexLocker locker(&m_mutex);
            m_errorString = u"Diagram source not found for hash"_s;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    MermaidRenderRequest req;
    req.code = code;
    req.isDarkMode = isDark;

    const QFont font = Config::editorFont();
    req.fontFamily = font.family().isEmpty() ? QFontDatabase::systemFont(QFontDatabase::GeneralFont).family() : font.family();
    if (req.fontFamily.isEmpty()) {
        req.fontFamily = u"sans-serif"_s;
    }
    req.fontSize = font.pointSizeF() > 0 ? font.pointSizeF() : (font.pixelSize() > 0 ? font.pixelSize() : 10.0);

    JsEngineMermaidRenderer::instance()->renderAsync(req, [this, isDark, req, id = m_id](const MermaidRenderResult &result) {
        if (m_cancelled.load(std::memory_order_relaxed)) {
            Q_EMIT finished();
            return;
        }

        if (!result.success) {
            {
                QMutexLocker locker(&m_mutex);
                m_errorString = result.errorString;
            }
            Q_EMIT finished();
            return;
        }

        QHash<QString, QString> customVars;
        if (!req.fontFamily.isEmpty()) {
            customVars[u"--pele-font"_s] = req.fontFamily;
        }
        QString flatSvg = MermaidCssFlattener::flattenSvg(result.svg, customVars, isDark);

        // Store flattened SVG in cache
        MermaidSvgCache::instance()->insert(id, flatSvg, result.naturalSize);

        QString err;
        QImage img = renderSvgToImage(flatSvg, result.naturalSize, err);
        {
            QMutexLocker locker(&m_mutex);
            m_image = img;
            m_errorString = err;
        }
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
