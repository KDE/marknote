// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../notebooksmodel.h"
#include <QAbstractItemModelTester>
#include <QObject>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class NotebooksModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + u"/.qttest/Notes";
        QDir(dir).removeRecursively();
    }

    void testNoteBookPath()
    {
        NoteBooksModel model(dir);
        auto tester = new QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(model.rowCount({}), 0);

        // Add
        model.addNoteBook(u"Test"_s, u"none"_s, u"#3f3f3f"_s);
        QCOMPARE(model.rowCount({}), 1);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Name).toString(), u"Test"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Color).toString(), u"#3f3f3f"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Icon).toString(), u"none"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Path).toString(), dir + u"/Test"_s);

        // Edit
        QSignalSpy spy(&model, &NoteBooksModel::noteBookRenamed);

        model.editNoteBook(dir + u"/Test"_s, u"NewTest"_s, u"none2"_s, u"#3f3f3a"_s);
        QCOMPARE(model.rowCount({}), 1);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Name).toString(), u"NewTest"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Color).toString(), u"#3f3f3a"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Icon).toString(), u"none2"_s);
        QCOMPARE(model.data(model.index(0, 0), NoteBooksModel::Role::Path).toString(), dir + u"/NewTest"_s);

        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).toString(), u"Test"_s);
        QCOMPARE(arguments.at(1).toString(), u"NewTest"_s);
        QCOMPARE(arguments.at(2).toString(), dir + u"/NewTest"_s);

        // Delete
        model.deleteNoteBook(dir + u"/NewTest"_s);
        QCOMPARE(model.rowCount({}), 0);
    }

private:
    QString dir;
};

QTEST_MAIN(NotebooksModelTest)
#include "notebooksmodeltest.moc"
