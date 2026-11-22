// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "jsenginemermaidrenderer.h"

#include <QEventLoop>
#include <QFile>
#include <QFont>
#include <QFontMetricsF>
#include <QJSEngine>
#include <QLoggingCategory>
#include <QMutex>

using namespace Qt::StringLiterals;

class NativeMeasurer : public QObject
{
    Q_OBJECT
public:
    explicit NativeMeasurer(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void setBaseFont(const QString &family, qreal pointSize)
    {
        m_baseFont.setFamily(family.isEmpty() ? u"sans-serif"_s : family);
        m_baseFont.setPointSizeF(pointSize > 0 ? pointSize : 10.0);
    }

    Q_INVOKABLE qreal width(const QString &text, qreal size, int style)
    {
        QFont font = m_baseFont;
        if (size > 0) {
            font.setPointSizeF(size);
        }
        if (style & 1) {
            font.setBold(true);
        }
        if (style & 2) {
            font.setItalic(true);
        }
        if (style & 4) {
            font.setFamily(u"monospace"_s);
        }
        QFontMetricsF fm(font);
        return fm.horizontalAdvance(text);
    }

private:
    QFont m_baseFont{u"sans-serif"_s, 10};
};

class MermaidWorker : public QObject
{
    Q_OBJECT
public:
    explicit MermaidWorker(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~MermaidWorker() override
    {
        delete m_measurer;
        delete m_engine;
    }

public Q_SLOTS:
    void init()
    {
        if (m_engine) {
            return;
        }

        Q_INIT_RESOURCE(pele);

        m_engine = new QJSEngine(this);
        m_engine->globalObject().setProperty(u"globalThis"_s, m_engine->globalObject());
        m_engine->globalObject().setProperty(u"window"_s, m_engine->globalObject());

        m_measurer = new NativeMeasurer(this);
        m_measurerJs = m_engine->newQObject(m_measurer);

        QFile file(u":/org/kde/marknote/mermaid/pele_bundle.js"_s);
        if (file.open(QIODevice::ReadOnly)) {
            QString script = QString::fromUtf8(file.readAll());
            file.close();
            QJSValue evalRes = m_engine->evaluate(script, u"pele_bundle.js"_s);
            if (evalRes.isError()) {
                m_initError = evalRes.toString();
            } else {
                m_pele = m_engine->globalObject().property(u"pele"_s);
                if (m_pele.isObject()) {
                    m_renderFn = m_pele.property(u"render"_s);
                }
            }
        } else {
            m_initError = u"Failed to open pele_bundle.js resource"_s;
        }
    }

    MermaidRenderResult doRender(const MermaidRenderRequest &request)
    {
        MermaidRenderResult result;

        if (!m_engine) {
            init();
        }

        if (!m_initError.isEmpty()) {
            result.errorString = m_initError;
            result.success = false;
            return result;
        }

        if (request.code.size() > 65536) {
            result.errorString = u"Diagram input exceeds 64KB limit."_s;
            result.success = false;
            return result;
        }

        m_measurer->setBaseFont(request.fontFamily, request.fontSize);

        QJSValue options = m_engine->newObject();
        options.setProperty(u"measurer"_s, m_measurerJs);
        options.setProperty(u"links"_s, false);
        options.setProperty(u"images"_s, false);
        options.setProperty(u"maxEdges"_s, 500);
        options.setProperty(u"outputLimit"_s, 500000);
        if (!request.fontFamily.isEmpty()) {
            options.setProperty(u"fontFamily"_s, request.fontFamily);
        }
        if (request.fontSize > 0) {
            options.setProperty(u"fontSize"_s, request.fontSize);
        }

        QJSValue res = m_renderFn.call(QJSValueList() << request.code << options);
        if (res.isError()) {
            result.errorString = res.toString();
            result.success = false;
        } else {
            result.svg = res.property(u"svg"_s).toString();
            int w = res.property(u"width"_s).toInt();
            int h = res.property(u"height"_s).toInt();
            result.naturalSize = QSize(w, h);
            result.success = true;
        }

        return result;
    }

private:
    QJSEngine *m_engine = nullptr;
    NativeMeasurer *m_measurer = nullptr;
    QJSValue m_measurerJs;
    QJSValue m_pele;
    QJSValue m_renderFn;
    QString m_initError;
};

JsEngineMermaidRenderer::JsEngineMermaidRenderer(QObject *parent)
    : MermaidRenderer(parent)
{
    QLoggingCategory::setFilterRules(u"qt.qml.usedbeforedeclared.warning=false\nqt.qml.usedbeforedeclared=false"_s);
    Q_INIT_RESOURCE(pele);

    m_worker = new MermaidWorker();
    m_worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::started, m_worker, &MermaidWorker::init);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread.start();
}

JsEngineMermaidRenderer::~JsEngineMermaidRenderer()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

JsEngineMermaidRenderer *JsEngineMermaidRenderer::instance()
{
    static JsEngineMermaidRenderer s_instance;
    return &s_instance;
}

void JsEngineMermaidRenderer::renderAsync(const MermaidRenderRequest &request, RenderCallback callback)
{
    QMetaObject::invokeMethod(m_worker, [this, request, callback]() {
        MermaidRenderResult result = m_worker->doRender(request);
        if (callback) {
            callback(result);
        }
    });
}

MermaidRenderResult JsEngineMermaidRenderer::renderSync(const MermaidRenderRequest &request)
{
    MermaidRenderResult syncResult;
    QEventLoop loop;
    renderAsync(request, [&syncResult, &loop](const MermaidRenderResult &res) {
        syncResult = res;
        loop.quit();
    });
    loop.exec();
    return syncResult;
}

#include "jsenginemermaidrenderer.moc"
