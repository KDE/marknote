// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../maildirimport.h"
#include <QObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class MaildirImportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testImport()
    {
        QTemporaryDir destination;

        MaildirImport maildirImport;
        maildirImport.import(QUrl::fromLocalFile(QLatin1StringView(DATA_DIR) + QLatin1StringView("/maildir/")), QUrl::fromLocalFile(destination.path()));

        QDir dir(destination.path());
        const auto entries = dir.entryInfoList(QDir::Files);
        QCOMPARE(entries.count(), 1);
        QFile file(entries[0].canonicalFilePath());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), "HELLO WORLD");
    }
};

QTEST_MAIN(MaildirImportTest)
#include "maildirimporttest.moc"
