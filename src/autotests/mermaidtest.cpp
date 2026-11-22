// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QTest>

#include "mermaid/jsenginemermaidrenderer.h"
#include "mermaid/mermaidcssflattener.h"
#include "mermaid/mermaidhelper.h"
#include "mermaid/mermaidimageprovider.h"

using namespace Qt::Literals::StringLiterals;

class MermaidTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() {
        Q_INIT_RESOURCE(pele);
    }
    void testLanguageDetection();
    void testSmokeDiagrams_data();
    void testSmokeDiagrams();
    void testCssFlattener();
    void testInvalidSyntax();
    void testInputLimit();
    void testCacheKey();
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
    QTest::newRow("FootnoteDiagram") << u"FootnoteDiagram"_s << u"flowchart TD\n    A[RFQ Issued & Quotes Received] --> B[L1 Quote Comparison Matrix]\n    B -->|Manufacturer Awards Quote| C[Atomic Transaction: PO Created]\n    Z[Manual Direct Procurement] -->|Manufacturer Creates Direct PO| C\n    \n    C -->|Status: Issued / Unpaid| D[Vendor Receives Order]\n    D -->|Vendor Acknowledges Order| E[Status: Confirmed / Acknowledged]"_s;
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

QTEST_MAIN(MermaidTest)

#include "mermaidtest.moc"
