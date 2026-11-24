// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QTest>

#include "mermaid/jsenginemermaidrenderer.h"
#include "mermaid/mermaidcssflattener.h"
#include "mermaid/mermaidhelper.h"
#include "mermaid/mermaidimageprovider.h"
#include "mermaid/mermaidsvgcache.h"
#include <marknotesettings.h>

using namespace Qt::Literals::StringLiterals;

class MermaidTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(pele);
    }
    void testLanguageDetection();
    void testSmokeDiagrams_data();
    void testSmokeDiagrams();
    void testCssFlattener();
    void testInvalidSyntax();
    void testInputLimit();
    void testCacheKey();
    void testDiagramWithTitle();
    void testSvgCacheHitMiss();
    void testSvgCacheEviction();
    void testNormalizeTspans();
};

void MermaidTest::testLanguageDetection()
{
    MermaidHelper helper;
    QVERIFY(helper.isMermaid(u"mermaid"_s, u"foo bar"_s));
    QVERIFY(helper.isMermaid(u"MMD"_s, u"foo bar"_s));
    QVERIFY(!helper.isMermaid(u"python"_s, u"flowchart TD\n  A --> B"_s));
    QVERIFY(!helper.isMermaid(u"cpp"_s, u"classDiagram"_s));

    QVERIFY(helper.isMermaid(QString(), u"flowchart TD\n  A --> B"_s));
    QVERIFY(helper.isMermaid(QString(), u"graph LR\n  A --> B"_s));
    QVERIFY(helper.isMermaid(QString(), u"sequenceDiagram\n  Alice->>Bob: Hi"_s));
    QVERIFY(helper.isMermaid(QString(), u"classDiagram\n  class A"_s));
    QVERIFY(helper.isMermaid(QString(), u"stateDiagram-v2\n  [*] --> Still"_s));
    QVERIFY(helper.isMermaid(QString(), u"erDiagram\n  A ||--o{ B : r"_s));
    QVERIFY(helper.isMermaid(QString(), u"gantt\n  title T\n  section S\n  A : 2026-01-01, 10d"_s));
    QVERIFY(helper.isMermaid(QString(), u"pie title Pets\n  \"Dogs\" : 10"_s));
    QVERIFY(helper.isMermaid(QString(), u"gitGraph\n  commit"_s));
    QVERIFY(helper.isMermaid(QString(), u"mindmap\n  root((R))"_s));
    QVERIFY(helper.isMermaid(QString(), u"radar-beta\n  axis food[\"Food\"]"_s));
    QVERIFY(helper.isMermaid(QString(), u"eventmodeling\n  timeframe 01 ui Cart"_s));

    QVERIFY(helper.isMermaid(QString(), u"%% A comment\nflowchart TD\n  A --> B"_s));

    QVERIFY(!helper.isMermaid(QString(), u"console.log('hello');"_s));
    QVERIFY(!helper.isMermaid(QString(), u"int main() { return 0; }"_s));
    QVERIFY(!helper.isMermaid(QString(), QString()));
}

void MermaidTest::testSmokeDiagrams_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("code");

    QTest::newRow("Flowchart") << u"Flowchart"_s << u"flowchart TD\n  A --> B\n  B --> C"_s;
    QTest::newRow("Sequence") << u"Sequence"_s << u"sequenceDiagram\n  Alice->>Bob: Hello\n  Bob-->>Alice: Hi"_s;
    QTest::newRow("Class") << u"Class"_s << u"classDiagram\n  class Animal {\n    +String name\n    +move()\n  }"_s;
    QTest::newRow("State") << u"State"_s << u"stateDiagram-v2\n  [*] --> Still\n  Still --> [*]"_s;
    QTest::newRow("ER") << u"ER"_s << u"erDiagram\n  CUSTOMER ||--o{ ORDER : places"_s;
    QTest::newRow("Gantt") << u"Gantt"_s << u"gantt\n  title Project\n  dateFormat YYYY-MM-DD\n  section A\n  Task 1: 2026-01-01, 30d"_s;
    QTest::newRow("Pie") << u"Pie"_s << u"pie title Pets\n  \"Dogs\" : 386\n  \"Cats\" : 85"_s;
    QTest::newRow("GitGraph") << u"GitGraph"_s << u"gitGraph\n  commit\n  branch develop\n  checkout develop\n  commit\n  checkout main\n  merge develop"_s;
    QTest::newRow("Mindmap") << u"Mindmap"_s << u"mindmap\n  root((Root))\n    Topic 1\n    Topic 2"_s;
    QTest::newRow("MindmapMarkdown")
        << u"MindmapMarkdown"_s
        << u"mindmap\n    id1[\"`**Pele** is the\ngoddess of volcanoes\nUnicode works too: 🌋`\"]\n      id2[\"`She lives in **the** crater of Kīlauea... a *very old story* that is still told today`\"]\n      id3[Lava builds new land]"_s;
    QTest::newRow("FootnoteDiagram")
        << u"FootnoteDiagram"_s
        << u"flowchart TD\n    A[RFQ Issued & Quotes Received] --> B[L1 Quote Comparison Matrix]\n    B -->|Manufacturer Awards Quote| C[Atomic Transaction: PO Created]\n    Z[Manual Direct Procurement] -->|Manufacturer Creates Direct PO| C\n    \n    C -->|Status: Issued / Unpaid| D[Vendor Receives Order]\n    D -->|Vendor Acknowledges Order| E[Status: Confirmed / Acknowledged]"_s;
}

void MermaidTest::testSmokeDiagrams()
{
    QFETCH(QString, name);
    QFETCH(QString, code);

    MermaidRenderRequest req;
    req.code = code;
    req.isDarkMode = false;
    req.fontFamily = u"sans-serif"_s;
    req.fontSize = 12.0;

    MermaidRenderResult res = JsEngineMermaidRenderer::instance()->renderSync(req);
    QVERIFY2(res.success, qPrintable(u"Rendering failed for %1: %2"_s.arg(name, res.errorString)));
    QVERIFY(!res.svg.isEmpty());
    QVERIFY(res.naturalSize.isValid());

    QString flatSvg = MermaidCssFlattener::flattenSvg(res.svg, {}, false);
    QSvgRenderer svgRenderer(flatSvg.toUtf8());
    QVERIFY2(svgRenderer.isValid(), qPrintable(u"QSvgRenderer rejected SVG for %1"_s.arg(name)));

    QSize sz = svgRenderer.defaultSize();
    if (sz.isEmpty()) {
        sz = res.naturalSize;
    }
    QImage img(sz, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    svgRenderer.render(&p);
    p.end();

    QVERIFY(!img.isNull());
    QVERIFY(img.width() > 0);
    QVERIFY(img.height() > 0);
}

void MermaidTest::testCssFlattener()
{
    const QString sample = QString::fromUtf8(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" style=\"--_bg:var(--pele-bg,#fff);--_fg:var(--pele-fg,#1f1f1f);color:var(--_fg)\">"
        "<rect x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"var(--_bg)\" stroke=\"currentColor\" style=\"rx:var(--_r)\"/>"
        "<text fill=\"var(--pele-series-1,#4c78a8)\">Hello</text>"
        "</svg>");

    QString flattened = MermaidCssFlattener::flattenSvg(sample, {}, false);

    QVERIFY(!flattened.contains(u"var(--_"_s));
    QVERIFY(!flattened.contains(u"var(--pele-"_s));
    QVERIFY(!flattened.contains(u"currentColor"_s));
    QVERIFY(flattened.contains(u"rx=\"4\""_s));

    QSvgRenderer renderer(flattened.toUtf8());
    QVERIFY(renderer.isValid());
}

void MermaidTest::testInvalidSyntax()
{
    MermaidRenderRequest req;
    req.code = u"flowchart TD\n  A --->>> ???"_s;
    MermaidRenderResult res = JsEngineMermaidRenderer::instance()->renderSync(req);

    QVERIFY(!res.success);
    QVERIFY(!res.errorString.isEmpty());
}

void MermaidTest::testInputLimit()
{
    MermaidRenderRequest req;
    req.code = QString(70000, QLatin1Char('A'));
    MermaidRenderResult res = JsEngineMermaidRenderer::instance()->renderSync(req);

    QVERIFY(!res.success);
    QVERIFY(res.errorString.contains(u"64KB limit"_s));
}

void MermaidTest::testCacheKey()
{
    MermaidHelper helper;
    const QString code = u"flowchart TD\n  A --> B"_s;

    QString key1 = helper.diagramId(code, false);
    QString key2 = helper.diagramId(code, false);
    QCOMPARE(key1, key2);

    QString keyDark = helper.diagramId(code, true);
    QVERIFY(key1 != keyDark);

    QString hash = key1.split(QLatin1Char('_')).at(0);
    QCOMPARE(MermaidImageProvider::getDiagram(hash), code);
}

void MermaidTest::testDiagramWithTitle()
{
    const QString code = u"---\ntitle: Comprehensive Project Architecture Plan 2026\n---\nflowchart TD\n  A --> B"_s;
    MermaidRenderRequest req;
    req.code = code;
    const QFont font = Config::editorFont();
    req.fontFamily = font.family();
    req.fontSize = font.pointSizeF() > 0 ? font.pointSizeF() : (font.pixelSize() > 0 ? font.pixelSize() : 12.0);

    MermaidRenderResult res = JsEngineMermaidRenderer::instance()->renderSync(req);
    QVERIFY(res.success);
    QVERIFY(!res.svg.isEmpty());

    QHash<QString, QString> customVars;
    if (!req.fontFamily.isEmpty()) {
        customVars[u"--pele-font"_s] = req.fontFamily;
    }
    QString flatSvg = MermaidCssFlattener::flattenSvg(res.svg, customVars, false);
    QSvgRenderer renderer(flatSvg.toUtf8());
    QVERIFY(renderer.isValid());

    QVERIFY(flatSvg.contains(u"Comprehensive Project Architecture Plan 2026"_s));

    QFont titleFont(req.fontFamily.isEmpty() ? u"sans-serif"_s : req.fontFamily);
    titleFont.setPointSizeF(req.fontSize);
    titleFont.setBold(true);
    QFontMetricsF fm(titleFont);
    qreal textWidth = fm.horizontalAdvance(u"Comprehensive Project Architecture Plan 2026"_s);
    QVERIFY(renderer.defaultSize().width() >= textWidth);
}

void MermaidTest::testSvgCacheHitMiss()
{
    MermaidSvgCache *cache = MermaidSvgCache::instance();
    cache->clear();
    QCOMPARE(cache->count(), 0);
    QCOMPARE(cache->totalCost(), 0);

    const QString testKey = u"test_hash_0_sans_10"_s;
    const QString sampleSvg = u"<svg viewBox=\"0 0 100 100\"><rect width=\"100\" height=\"100\"/></svg>"_s;
    const QSize sampleSize(100, 100);

    QVERIFY(!cache->contains(testKey));
    QVERIFY(cache->get(testKey) == nullptr);

    cache->insert(testKey, sampleSvg, sampleSize);

    QVERIFY(cache->contains(testKey));
    QCOMPARE(cache->count(), 1);
    QVERIFY(cache->totalCost() > 0);

    auto item = cache->get(testKey);
    QVERIFY(item != nullptr);
    QCOMPARE(item->flatSvg, sampleSvg);
    QCOMPARE(item->naturalSize, sampleSize);
    QCOMPARE(item->cost(), static_cast<int>(sizeof(CachedSvgDiagram)) + (sampleSvg.size() * static_cast<int>(sizeof(QChar))));

    cache->clear();
    QCOMPARE(cache->count(), 0);
    QCOMPARE(cache->totalCost(), 0);
}

void MermaidTest::testSvgCacheEviction()
{
    MermaidSvgCache *cache = MermaidSvgCache::instance();
    cache->clear();

    const int originalMaxCost = cache->maxCost();
    cache->setMaxCost(150);
    QCOMPARE(cache->maxCost(), 150);

    const QString svgSmall1 = u"<svg a='1'></svg>"_s;
    const QString svgSmall2 = u"<svg b='2'></svg>"_s;
    const QString svgSmall3 = u"<svg c='3'></svg>"_s;

    cache->insert(u"key1"_s, svgSmall1, QSize(10, 10));
    cache->insert(u"key2"_s, svgSmall2, QSize(10, 10));
    QVERIFY(cache->contains(u"key1"_s));
    QVERIFY(cache->contains(u"key2"_s));

    // Access key1 to make key2 least recently used
    cache->get(u"key1"_s);

    // Inserting key3 exceeds 150 bytes and evicts key2 (LRU)
    cache->insert(u"key3"_s, svgSmall3, QSize(10, 10));

    QVERIFY(cache->totalCost() <= 150);
    QVERIFY(cache->contains(u"key1"_s));
    QVERIFY(cache->contains(u"key3"_s));
    QVERIFY(!cache->contains(u"key2"_s));

    cache->setMaxCost(originalMaxCost);
    cache->clear();
}

void MermaidTest::testNormalizeTspans()
{
    // Single tspan with x and y
    const QString single = u"<text class=\"title\"><tspan x=\"10\" y=\"20\">Title</tspan></text>"_s;
    const QString normSingle = MermaidCssFlattener::normalizeTspans(single);
    QVERIFY(!normSingle.contains(u"<tspan"_s));
    QVERIFY(normSingle.contains(u"x=\"10\""_s));
    QVERIFY(normSingle.contains(u"y=\"20\""_s));
    QVERIFY(normSingle.contains(u"class=\"title\""_s));
    QVERIFY(normSingle.contains(u">Title</text>"_s));

    // Multi-line tspan
    const QString multi = u"<text class=\"c4-descr\" font-size=\"13\"><tspan x=\"0\" y=\"10\">Line 1</tspan><tspan x=\"0\" y=\"25\">Line 2</tspan></text>"_s;
    const QString normMulti = MermaidCssFlattener::normalizeTspans(multi);
    QVERIFY(!normMulti.contains(u"<tspan"_s));
    QVERIFY(normMulti.contains(u"<text class=\"c4-descr\" font-size=\"13\" x=\"0\" y=\"10\">Line 1</text>"_s));
    QVERIFY(normMulti.contains(u"<text class=\"c4-descr\" font-size=\"13\" x=\"0\" y=\"25\">Line 2</text>"_s));

    // Inline tspan without y should NOT be transformed
    const QString inlineTspan = u"<text x=\"10\" y=\"20\">Total: <tspan class=\"val\">42</tspan></text>"_s;
    const QString normInline = MermaidCssFlattener::normalizeTspans(inlineTspan);
    QCOMPARE(normInline, inlineTspan);

    const QString mdText =
        u"<text class=\"pele-label\" text-anchor=\"middle\"><tspan x=\"0\" y=\"-16\">She lives in </tspan><tspan font-weight=\"bold\">the</tspan><tspan> "
        u"crater</tspan><tspan x=\"0\" y=\"3\">of Kilauea</tspan></text>"_s;
    const QString normMd = MermaidCssFlattener::normalizeTspans(mdText);
    QVERIFY(
        normMd.contains(u"<text class=\"pele-label\" text-anchor=\"middle\" x=\"0\" xml:space=\"preserve\" y=\"-16\"><tspan>She lives in </tspan><tspan "
                        u"font-weight=\"bold\">the</tspan><tspan> "
                        u"crater</tspan></text>"_s));
    QVERIFY(normMd.contains(u"<text class=\"pele-label\" text-anchor=\"middle\" x=\"0\" y=\"3\">of Kilauea</text>"_s));

    const QString formattedText =
        u"<text class=\"pele-label\" text-anchor=\"middle\"><tspan x=\"0\" y=\"0\">Normal </tspan><tspan font-weight=\"bold\">Bold</tspan><tspan> </tspan><tspan font-style=\"italic\">Italic</tspan><tspan x=\"0\" y=\"15\" text-decoration=\"underline\">Underline</tspan></text>"_s;
    const QString normFormatted = MermaidCssFlattener::normalizeTspans(formattedText);
    QVERIFY(normFormatted.contains(u"font-style=\"italic\""_s));
    QVERIFY(normFormatted.contains(u"font-weight=\"bold\""_s));
    QVERIFY(normFormatted.contains(u"text-decoration=\"underline\""_s));
    QVERIFY(normFormatted.contains(u"y=\"15\""_s));
    QVERIFY(normFormatted.contains(u"xml:space=\"preserve\""_s));
}

QTEST_MAIN(MermaidTest)

#include "mermaidtest.moc"
