// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commands/commandmanager.h"
#include "mdtreemodel/mdtreemodel.h"
#include "mdtreemodel/treeitem.h"
#include <QObject>
#include <QTemporaryFile>
#include <QtTest/QtTest>
#include <md4qt/parser.h>

using namespace Qt::Literals::StringLiterals;

class TreeModelSerializerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTreeItemMutationSync()
    {
        TreeItem *list = TreeItem::createTreeItem(MDOptions::ElementType::List);
        QVERIFY(list != nullptr);
        auto mdList = list->itemAs<MD::List>();
        QVERIFY(mdList != nullptr);

        // createTreeItem for List creates 1 default child ListItem
        QCOMPARE(list->childCount(), 1);
        QCOMPARE(mdList->items().size(), 1);

        // Add a second ListItem
        TreeItem *item2 = TreeItem::createTreeItem(MDOptions::ElementType::ListItem, u"Second item"_s);
        list->appendChild(item2);

        QCOMPARE(list->childCount(), 2);
        QCOMPARE(mdList->items().size(), 2);
        QCOMPARE(mdList->items().at(1), item2->item());

        // Remove the first child
        TreeItem *removed = list->removeChild(0);
        QVERIFY(removed != nullptr);
        QCOMPARE(list->childCount(), 1);
        QCOMPARE(mdList->items().size(), 1);
        QCOMPARE(mdList->items().at(0), item2->item());
        delete removed;

        delete list;
    }

    void testCommitUnparsedMarkdown()
    {
        TreeItem *paragraph = TreeItem::createTreeItem(MDOptions::ElementType::Paragraph, u"Initial"_s);
        QCOMPARE(paragraph->data()[u"md"_s].toString(), u"Initial"_s);

        paragraph->setUnparsedMarkdown(u"Updated **bold** text"_s);
        QCOMPARE(paragraph->data()[u"md"_s].toString(), u"Updated **bold** text"_s);

        paragraph->commitUnparsedMarkdown();
        QVERIFY(paragraph->unparsedMarkdown().isNull());
        QCOMPARE(paragraph->data()[u"md"_s].toString(), u"Updated **bold** text"_s);

        delete paragraph;
    }

    void testModelSerialization()
    {
        MDTreeModel model;
        TreeItem *root = model.rootItem();
        QVERIFY(root != nullptr);

        TreeItem *heading = TreeItem::createTreeItem(MDOptions::ElementType::Heading, u"# Title"_s);
        model.insertItem(root, 0, heading);

        TreeItem *para = TreeItem::createTreeItem(MDOptions::ElementType::Paragraph, u"A paragraph with text."_s);
        model.insertItem(root, 1, para);

        TreeItem *code = TreeItem::createTreeItem(MDOptions::ElementType::Code, u"int x = 42;"_s);
        model.insertItem(root, 2, code);

        QString markdown = model.toMarkdown();
        QVERIFY(markdown.contains(u"# Title"_s));
        QVERIFY(markdown.contains(u"A paragraph with text."_s));
        QVERIFY(markdown.contains(u"int x = 42;"_s));
    }

    void testSaveToFileAndCommandManager()
    {
        MDTreeModel model;
        TreeItem *root = model.rootItem();

        TreeItem *heading = TreeItem::createTreeItem(MDOptions::ElementType::Heading, u"# My Note"_s);
        model.insertItem(root, 0, heading);

        TreeItem *para = TreeItem::createTreeItem(MDOptions::ElementType::Paragraph, u"Sample content"_s);
        model.insertItem(root, 1, para);

        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QString tempPath = tempFile.fileName();
        tempFile.close();

        QUrl fileUrl = QUrl::fromLocalFile(tempPath);

        CommandManager manager;
        manager.setModel(&model);

        QVERIFY(manager.saveFile(fileUrl));

        QFile savedFile(tempPath);
        QVERIFY(savedFile.open(QFile::ReadOnly));
        QString savedContent = QString::fromUtf8(savedFile.readAll());
        savedFile.close();

        QVERIFY(savedContent.contains(u"# My Note"_s));
        QVERIFY(savedContent.contains(u"Sample content"_s));
    }

    void testListAndBlockquoteSerialization()
    {
        MDTreeModel model;
        TreeItem *root = model.rootItem();

        TreeItem *blockquote = TreeItem::createTreeItem(MDOptions::ElementType::Blockquote, u"A wise quote"_s);
        model.insertItem(root, 0, blockquote);

        auto listItems = TreeItem::fromMarkdown(u"- First item\n- Second item\n"_s);
        QVERIFY(!listItems.isEmpty());
        model.insertItem(root, 1, listItems.first());

        QString markdown = model.toMarkdown();
        QVERIFY(markdown.contains(u"> A wise quote"_s));
        QVERIFY(markdown.contains(u"- First item"_s));
        QVERIFY(markdown.contains(u"- Second item"_s));
    }

    void testTableSerialization()
    {
        MDTreeModel model;
        TreeItem *root = model.rootItem();

        auto tableItems = TreeItem::fromMarkdown(u"| A | B |\n| --- | --- |\n| 1 | 2 |\n"_s);
        QVERIFY(!tableItems.isEmpty());
        TreeItem *table = tableItems.first();
        model.insertItem(root, 0, table);

        model.setItemTableMD(table, 0, 0, u"Alpha"_s);
        model.setItemTableMD(table, 1, 1, u"Two"_s);

        QString markdown = model.toMarkdown();
        QVERIFY(markdown.contains(u"Alpha"_s));
        QVERIFY(markdown.contains(u"Two"_s));
    }

    void testRoundtripDocument()
    {
        const QString originalMd =
            u"# Main Header\n\nThis is a paragraph with **bold** text.\n\n- item 1\n- item 2\n\n> Quote here\n\n```cpp\nint main() { return 0; }\n```\n"_s;

        QTextStream stream{originalMd.toUtf8()};
        MD::Parser parser;
        auto doc = parser.parse(stream, u""_s, u""_s);

        MDTreeModel model;
        model.setDocument(doc);

        QString serializedMd = model.toMarkdown();
        QVERIFY(serializedMd.contains(u"# Main Header"_s));
        QVERIFY(serializedMd.contains(u"This is a paragraph with **bold** text."_s));
        QVERIFY(serializedMd.contains(u"item 1"_s));
        QVERIFY(serializedMd.contains(u"item 2"_s));
        QVERIFY(serializedMd.contains(u"> Quote here"_s));
        QVERIFY(serializedMd.contains(u"int main() { return 0; }"_s));
    }
};

QTEST_MAIN(TreeModelSerializerTest)
#include "treemodelserializertest.moc"
