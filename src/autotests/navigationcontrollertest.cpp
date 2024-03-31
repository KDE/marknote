// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../navigationcontroller.h"
#include <QObject>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class NavigationControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testNoteBookPath()
    {
        NavigationController controller;
        QSignalSpy spy(&controller, &NavigationController::notebookPathChanged);

        controller.setNotebookPath(u"path/to/Name"_s);
        QCOMPARE(controller.notebookPath(), u"path/to/Name"_s);
        QCOMPARE(controller.notebookName(), u"Name"_s);

        controller.setNotebookPath(u"path/to/Name"_s);
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(NavigationControllerTest)
#include "navigationcontrollertest.moc"
