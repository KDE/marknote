// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../nestedlisthelper_p.h"
#include <QObject>
#include <QTextCursor>
#include <QTextDocument>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class NestedListHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testIndentDetentSimple()
    {
        NestedListHelper helper;
        QTextDocument doc;
        doc.setMarkdown(u"Hello\n"_s);

        QTextCursor cursor(&doc);
        QCOMPARE(cursor.position(), 0);

        QVERIFY(!helper.canDedent(cursor));

        helper.handleOnIndentMore(cursor);

        QVERIFY(helper.canDedent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"- Hello");

        helper.handleOnIndentLess(cursor);

        QVERIFY(!helper.canDedent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"Hello");
    }

    void testIndentDetentNested()
    {
        NestedListHelper helper;
        QTextDocument doc;
        doc.setMarkdown(u"- Hello\n  - Hello"_s);

        QTextCursor cursor(&doc);
        QCOMPARE(cursor.position(), 0);

        QVERIFY(!helper.canDedent(cursor));
        QVERIFY(!helper.canIndent(cursor));

        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        QCOMPARE(cursor.position(), 6);

        QVERIFY(helper.canDedent(cursor));
        QVERIFY(!helper.canIndent(cursor));

        helper.handleOnIndentLess(cursor);

        QVERIFY(helper.canDedent(cursor));
        QVERIFY(helper.canIndent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"- Hello\n- Hello");

        helper.handleOnIndentLess(cursor);

        QVERIFY(!helper.canDedent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"- Hello\n\nHello");

        helper.handleOnIndentMore(cursor);

        QVERIFY(helper.canDedent(cursor));
        QVERIFY(helper.canIndent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"- Hello\n- Hello");

        helper.handleOnIndentMore(cursor);

        QVERIFY(helper.canDedent(cursor));
        QVERIFY(!helper.canIndent(cursor));
        QCOMPARE(doc.toMarkdown().trimmed(), u"- Hello\n  - Hello");
    }
};

QTEST_MAIN(NestedListHelperTest)
#include "nestedlisthelpertest.moc"
