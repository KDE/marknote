// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidimageprovider.h"
#include "jsenginemermaidrenderer.h"
#include "mermaidcssflattener.h"
#include "mermaidsvgcache.h"
#include <marknotesettings.h>

#include <QCache>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QSvgRenderer>
#include <QThread>

using namespace Qt::StringLiterals;

namespace
{
QCache<QString, QString> &diagramCodeRegistry()
{
    static QCache<QString, QString> s_registry(4 * 1024 * 1024);
    return s_registry;
}

QCache<QString, MermaidRenderContext> &renderContextRegistry()
{
    static QCache<QString, MermaidRenderContext> s_contexts(256);
    return s_contexts;
}

QMutex &registryMutex()
{
    static QMutex s_regMutex;
    return s_regMutex;
}

MermaidRenderContext resolveContextOnGuiThread(bool isDark)
{
    MermaidRenderContext ctx;
    auto resolve = [&ctx, isDark]() {
        const QFont font = Config::editorFont();
        ctx.fontFamily = font.family().isEmpty() ? QFontDatabase::systemFont(QFontDatabase::GeneralFont).family() : font.family();
        if (ctx.fontFamily.isEmpty()) {
            ctx.fontFamily = u"sans-serif"_s;
        }
        ctx.fontSize = font.pointSizeF() > 0 ? font.pointSizeF() : (font.pixelSize() > 0 ? font.pixelSize() : 10.0);
        ctx.variables = MermaidCssFlattener::defaultVariables(isDark);
        ctx.variables[u"--pele-font"_s] = ctx.fontFamily;
    };

    if (qApp && QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(qApp, resolve, Qt::BlockingQueuedConnection);
    } else {
        resolve();
    }
    return ctx;
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
    , m_state(std::make_shared<SharedState>())
{
    startJob();
}

MermaidImageResponse::~MermaidImageResponse()
{
    QMutexLocker locker(&m_state->mutex);
    m_state->cancelled = true;
    m_state->destroyed = true;
}

QQuickTextureFactory *MermaidImageResponse::textureFactory() const
{
    QMutexLocker locker(&m_state->mutex);
    return QQuickTextureFactory::textureFactoryForImage(m_state->image);
}

QString MermaidImageResponse::errorString() const
{
    QMutexLocker locker(&m_state->mutex);
    return m_state->errorString;
}

void MermaidImageResponse::cancel()
{
    QMutexLocker locker(&m_state->mutex);
    m_state->cancelled = true;
}

void MermaidImageResponse::startJob()
{
    {
        QMutexLocker locker(&m_state->mutex);
        if (m_state->cancelled) {
            QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
            return;
        }
    }

    const QStringList parts = m_id.split(QLatin1Char('_'));
    if (parts.isEmpty()) {
        {
            QMutexLocker locker(&m_state->mutex);
            m_state->errorString = u"Invalid diagram image ID format"_s;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QString hash = parts.at(0);
    const bool isDark = (parts.value(1) == u"1"_s);

    if (auto cached = MermaidSvgCache::instance()->get(m_id)) {
        QString err;
        QImage img = renderSvgToImage(cached->flatSvg, cached->naturalSize, err);
        {
            QMutexLocker locker(&m_state->mutex);
            m_state->image = img;
            m_state->errorString = err;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    const QString code = MermaidImageProvider::getDiagram(hash);
    if (code.isEmpty()) {
        {
            QMutexLocker locker(&m_state->mutex);
            m_state->errorString = u"Diagram source not found for hash"_s;
        }
        QMetaObject::invokeMethod(this, &QQuickImageResponse::finished, Qt::QueuedConnection);
        return;
    }

    MermaidRenderContext ctx = MermaidImageProvider::getRenderContext(m_id);
    if (ctx.variables.isEmpty()) {
        ctx = resolveContextOnGuiThread(isDark);
    }

    MermaidRenderRequest req;
    req.code = code;
    req.isDarkMode = isDark;
    req.fontFamily = ctx.fontFamily;
    req.fontSize = ctx.fontSize;
    req.colorOverrides = ctx.variables;

    const auto state = m_state;
    MermaidImageResponse *rawThis = this;

    JsEngineMermaidRenderer::instance()->renderAsync(req, [rawThis, state, vars = std::move(ctx.variables), id = m_id](const MermaidRenderResult &result) {
        {
            QMutexLocker locker(&state->mutex);
            if (state->cancelled) {
                if (!state->destroyed) {
                    QMetaObject::invokeMethod(rawThis, &QQuickImageResponse::finished, Qt::QueuedConnection);
                }
                return;
            }
        }

        if (!result.success) {
            QMutexLocker locker(&state->mutex);
            state->errorString = result.errorString;
            if (!state->destroyed) {
                QMetaObject::invokeMethod(rawThis, &QQuickImageResponse::finished, Qt::QueuedConnection);
            }
            return;
        }

        const QString flatSvg = MermaidCssFlattener::flattenSvgResolved(result.svg, vars);
        MermaidSvgCache::instance()->insert(id, flatSvg, result.naturalSize);

        QString err;
        QImage img = renderSvgToImage(flatSvg, result.naturalSize, err);

        QMutexLocker locker(&state->mutex);
        state->image = img;
        state->errorString = err;
        if (!state->destroyed) {
            QMetaObject::invokeMethod(rawThis, &QQuickImageResponse::finished, Qt::QueuedConnection);
        }
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
    const int cost = qMax(1, static_cast<int>(code.size() * sizeof(QChar)));
    diagramCodeRegistry().insert(hash, new QString(code), cost);
}

QString MermaidImageProvider::getDiagram(const QString &hash)
{
    QMutexLocker locker(&registryMutex());
    if (const QString *code = diagramCodeRegistry().object(hash)) {
        return *code;
    }
    return QString();
}

void MermaidImageProvider::registerRenderContext(const QString &diagramId, const MermaidRenderContext &context)
{
    QMutexLocker locker(&registryMutex());
    renderContextRegistry().insert(diagramId, new MermaidRenderContext(context), 1);
}

MermaidRenderContext MermaidImageProvider::getRenderContext(const QString &diagramId)
{
    QMutexLocker locker(&registryMutex());
    if (const MermaidRenderContext *ctx = renderContextRegistry().object(diagramId)) {
        return *ctx;
    }
    return {};
}
