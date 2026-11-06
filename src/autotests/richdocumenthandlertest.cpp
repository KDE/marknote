// SPDX-FileCopyrightText: 2026 Partha Pratim Gogoi <yes.par781@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "../mdtreemodel/mdtreemodel.h"
#include "../rich_documenthandler.h"
#include <QFile>
#include <QObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class RichDocumentHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSaveBeforeLoadFinishedKeepsFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(u"note.md"_s);
        writeFile(path, "# Title\n\nSome content\n");

        RichDocumentHandler handler;
        const QUrl url = QUrl::fromLocalFile(path);

        handler.load(url);
        // The parse result is not delivered yet. The model is still the empty placeholder.
        handler.saveAs(url);
        QCOMPARE(readFile(path), QByteArray("# Title\n\nSome content\n"));

        QTRY_VERIFY(handler.treeModel()->toMarkdown().contains(u"Some content"_s));
        handler.saveAs(url);
        QVERIFY(readFile(path).contains("Some content"));
    }

    void testSwitchingNotesQuicklyKeepsBothFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString pathA = dir.filePath(u"a.md"_s);
        const QString pathB = dir.filePath(u"b.md"_s);
        writeFile(pathA, "Note A\n");
        writeFile(pathB, "Note B\n");

        RichDocumentHandler handler;
        const QUrl urlA = QUrl::fromLocalFile(pathA);
        const QUrl urlB = QUrl::fromLocalFile(pathB);

        handler.load(urlA);
        handler.load(urlB);
        handler.saveAs(urlA);
        handler.saveAs(urlB);
        QCOMPARE(readFile(pathA), QByteArray("Note A\n"));
        QCOMPARE(readFile(pathB), QByteArray("Note B\n"));
    }

    void testEmptiedNoteStillSaves()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(u"note.md"_s);
        writeFile(path, "Some content\n");

        RichDocumentHandler handler;
        const QUrl url = QUrl::fromLocalFile(path);

        handler.load(url);
        QTRY_VERIFY(handler.treeModel()->toMarkdown().contains(u"Some content"_s));

        handler.treeModel()->setDocument(nullptr);
        handler.saveAs(url);
        QVERIFY(!readFile(path).contains("Some content"));
    }

private:
    static void writeFile(const QString &path, const QByteArray &content)
    {
        QFile file(path);
        QVERIFY(file.open(QFile::WriteOnly));
        file.write(content);
    }

    static QByteArray readFile(const QString &path)
    {
        QFile file(path);
        return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
    }
};

QTEST_MAIN(RichDocumentHandlerTest)
#include "richdocumenthandlertest.moc"
