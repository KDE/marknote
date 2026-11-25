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
#include <QRegularExpression>

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
    void resetEngine()
    {
        delete m_measurer;
        m_measurer = nullptr;
        delete m_engine;
        m_engine = nullptr;
        m_mathJaxLoaded = false;
        m_initError.clear();
        init();
    }

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
        QJSValue measurerJs = m_engine->newQObject(m_measurer);
        m_engine->globalObject().setProperty(u"__measurer"_s, measurerJs);

        static const QString polyfills = QStringLiteral(R"(
            if (!Array.prototype.flat) {
                Array.prototype.flat = function(depth) {
                    var d = depth === undefined ? 1 : Math.floor(depth);
                    if (d < 1) return Array.prototype.slice.call(this);
                    return (function flat(arr, currDepth) {
                        var res = [];
                        for (var i = 0; i < arr.length; i++) {
                            if (Array.isArray(arr[i]) && currDepth < d) {
                                res.push.apply(res, flat(arr[i], currDepth + 1));
                            } else if (arr[i] !== undefined) {
                                res.push(arr[i]);
                            }
                        }
                        return res;
                    })(this, 0);
                };
            }
            if (!String.prototype.replaceAll) {
                String.prototype.replaceAll = function(search, replacement) {
                    if (search instanceof RegExp) {
                        return this.replace(search, replacement);
                    }
                    return this.split(search).join(replacement);
                };
            }
            if (typeof Int32Array !== "undefined" && !Int32Array.prototype.sort) {
                var _types = [Int8Array, Uint8Array, Uint8ClampedArray, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array];
                for (var i = 0; i < _types.length; i++) {
                    if (_types[i] && !_types[i].prototype.sort) {
                        _types[i].prototype.sort = Array.prototype.sort;
                    }
                }
            }
        )");
        m_engine->evaluate(polyfills, u"polyfills.js"_s);

        static QString s_script;
        if (s_script.isEmpty()) {
            QFile file(u":/org/kde/marknote/mermaid/pele_bundle.js"_s);
            if (file.open(QIODevice::ReadOnly)) {
                s_script = QString::fromUtf8(file.readAll());
                file.close();
            }
        }

        if (!s_script.isEmpty()) {
            QJSValue evalRes = m_engine->evaluate(s_script, u"pele_bundle.js"_s);
            if (evalRes.isError()) {
                m_initError = evalRes.toString();
            }
        } else {
            m_initError = u"Failed to open pele_bundle.js resource"_s;
        }
    }

    MermaidRenderResult doRender(const MermaidRenderRequest &request)
    {
        MermaidRenderResult result;

        resetEngine();

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
        options.setProperty(u"measurer"_s, m_engine->globalObject().property(u"__measurer"_s));
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

        QString code = request.code;
        if (code.contains(u"$$"_s)) {
            ensureMathJaxLoaded();
        }
        // Work around upstream Pele 1.13.0 radar-beta parser bug where in-body 'title' is omitted from grammar
        if (code.contains(u"radar-beta"_s) && !code.trimmed().startsWith(u"---"_s)) {
            static const QRegularExpression radarTitleRegex(u"(?:\\A|\\n)[\\t ]*title[\\t ]+([^\\n\\r]+)"_s);
            auto match = radarTitleRegex.match(code);
            if (match.hasMatch()) {
                const QString titleText = match.captured(1).trimmed();
                code.remove(match.capturedStart(), match.capturedLength());
                code = u"---\ntitle: \"%1\"\n---\n%2"_s.arg(titleText, code);
            }
        }

        QJSValue pele = m_engine->globalObject().property(u"pele"_s);
        QJSValue renderFn = pele.property(u"render"_s);
        QJSValue res = renderFn.call(QJSValueList() << code << options);
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

        m_engine->collectGarbage();
        return result;
    }

private:
    void ensureMathJaxLoaded()
    {
        if (m_mathJaxLoaded || !m_engine) {
            return;
        }
        m_mathJaxLoaded = true;

        static QString s_mathJaxScript;
        if (s_mathJaxScript.isEmpty()) {
            QFile file(u":/org/kde/marknote/mermaid/mathjax_bundle.js"_s);
            if (file.open(QIODevice::ReadOnly)) {
                s_mathJaxScript = QString::fromUtf8(file.readAll());
                file.close();
            }
        }

        if (!s_mathJaxScript.isEmpty()) {
            m_engine->evaluate(s_mathJaxScript, u"mathjax_bundle.js"_s);
        }
    }

    QJSEngine *m_engine = nullptr;
    NativeMeasurer *m_measurer = nullptr;
    QString m_initError;
    bool m_mathJaxLoaded = false;
};

JsEngineMermaidRenderer::JsEngineMermaidRenderer(QObject *parent)
    : MermaidRenderer(parent)
{
    QLoggingCategory::setFilterRules(u"qt.qml.usedbeforedeclared.warning=false\nqt.qml.usedbeforedeclared=false"_s);
    Q_INIT_RESOURCE(pele);

    m_worker = new MermaidWorker();
    m_worker->moveToThread(&m_workerThread);

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
