// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mddatagenerator.h"
#include "mdoptions/mdoptions.h"
#include <md4qt/html.h>
using namespace Qt::StringLiterals;

namespace MDDataGenerator
{
QString toHtml(const QSharedPointer<MD::Item> &item)
{
    auto doc = QSharedPointer<MD::Document>::create();
    doc->appendItem(item);

    return MD::toHtml(doc, false, {}, false);
}

QVariantMap fromHeading(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;

    const auto heading = item.dynamicCast<MD::Heading>();

    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Heading);
    map[u"level"_s] = heading->level();
    map[u"html"_s] = toHtml(item);

    return map;
}

QVariantMap fromParagraph(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Paragraph);
    map[u"html"_s] = toHtml(item);

    return map;
}

QVariantMap fromBlockquote(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Blockquote);
    return map;
}

QVariantMap fromListItem(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::ListItem);

    const auto listItem = item.dynamicCast<MD::ListItem>();
    if (listItem->isTaskList()) {
        map[u"listType"_s] = QVariant::fromValue(MDOptions::ListType::TaskList);
        map[u"isChecked"_s] = QVariant::fromValue(listItem->isChecked());
    } else {
        if (listItem->listType() == MD::ListItem::Ordered) {
            map[u"listType"_s] = QVariant::fromValue(MDOptions::ListType::OrderedList);
        } else {
            map[u"listType"_s] = QVariant::fromValue(MDOptions::ListType::UnorderedList);
        }
    }

    return map;
}

QVariantMap fromList(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::List);
    return map;
}

QVariantMap fromCodeBlock(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Code);

    const auto code = item.dynamicCast<MD::Code>();

    map[u"lang"_s] = QVariant::fromValue(code->syntax());
    map[u"text"_s] = QVariant::fromValue(code->text());

    return map;
}

QVariantMap fromTable(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Table);

    auto table = item.dynamicCast<MD::Table>();

    map[u"rowCount"_s] = QVariant::fromValue(table->rows().size());
    map[u"columnCount"_s] = QVariant::fromValue(table->columnsCount());

    QList<QVariantList> data;

    for (const auto &row : table->rows()) {
        QVariantList rowData;

        for (const auto &cell : row->cells()) {
            // toHtml does not work for tableCells
            // so we need a paragraph instead
            auto p = QSharedPointer<MD::Paragraph>::create();

            for (const auto &item : cell->items()) {
                p->appendItem(item);
            }

            rowData.append(toHtml(p));
        }

        data.append(rowData);
    }

    map[u"htmlData"_s] = QVariant::fromValue(data);

    return map;
}

QVariantMap fromFootnote(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Footnote);
    return map;
}

QVariantMap fromDocument(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Document);
    return map;
}

QVariantMap fromPageBreak(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::PageBreak);
    return map;
}

QVariantMap fromAnchor(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::Anchor);
    return map;
}

QVariantMap fromHorizontalLine(const QSharedPointer<MD::Item> &item)
{
    QVariantMap map;
    map[u"type"_s] = QVariant::fromValue(MDOptions::ElementType::HorizontalLine);
    return map;
}
};
