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

    void testSaveToUnloadedUrlKeepsOtherNote()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString pathA = dir.filePath(u"a.md"_s);
        const QString pathB = dir.filePath(u"b.md"_s);
        writeFile(pathA, "Note A\n");
        writeFile(pathB, "Note B\n");

        RichDocumentHandler handler;
        handler.load(QUrl::fromLocalFile(pathA));
        QTRY_VERIFY(handler.treeModel()->toMarkdown().contains(u"Note A"_s));

        // This handler never loaded note B. Its model holds note A.
        handler.saveAs(QUrl::fromLocalFile(pathB));
        QCOMPARE(readFile(pathB), QByteArray("Note B\n"));
    }

    void testUnchangedNoteIsNotRewritten_data()
    {
        QTest::addColumn<QByteArray>("markdown");

        QTest::newRow("dash list") << QByteArray("- one\n- two\n");
        QTest::newRow("star list") << QByteArray("* one\n* two\n");
        QTest::newRow("ordered list") << QByteArray("1. one\n2. two\n");
        QTest::newRow("task list") << QByteArray("- [ ] todo\n- [x] done\n");
        QTest::newRow("headings") << QByteArray("# One\n\n## Two\n\nText\n");
        QTest::newRow("emphasis") << QByteArray("**bold** and *italic* and `code`\n");
        QTest::newRow("link") << QByteArray("[KDE](https://kde.org)\n");
        QTest::newRow("code block") << QByteArray("```cpp\nint main() {}\n```\n");
        QTest::newRow("blockquote") << QByteArray("> quoted\n> text\n");
        QTest::newRow("table") << QByteArray("| a | b |\n|---|---|\n| 1 | 2 |\n");
        QTest::newRow("rule") << QByteArray("one\n\n---\n\ntwo\n");
        QTest::newRow("no trailing newline") << QByteArray("plain text");
    }

    void testUnchangedNoteIsNotRewritten()
    {
        QFETCH(QByteArray, markdown);

        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(u"note.md"_s);
        writeFile(path, markdown);

        RichDocumentHandler handler;
        const QUrl url = QUrl::fromLocalFile(path);

        handler.load(url);
        QSignalSpy parsed(handler.treeModel(), &QAbstractItemModel::modelReset);
        QVERIFY(parsed.wait(5000));

        // The serializer would reformat some of these notes. Nothing changed, so the file must stay as it is.
        handler.saveAs(url);
        QCOMPARE(readFile(path), markdown);
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
