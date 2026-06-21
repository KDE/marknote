// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDOPTIONS_H
#define MDOPTIONS_H

#include <QObject>
#include <QQmlEngine>

namespace MDOptions
{
Q_NAMESPACE
QML_ELEMENT

enum ElementType {
    Heading = 0,
    Text = 1,
    Paragraph = 2,
    LineBreak = 3,
    Blockquote = 4,
    ListItem = 5,
    List = 6,
    Link = 7,
    Image = 8,
    Code = 9,
    TableCell = 10,
    TableRow = 11,
    Table = 12,
    FootnoteRef = 13,
    Footnote = 14,
    Document = 15,
    PageBreak = 16,
    Anchor = 17,
    HorizontalLine = 18,
    RawHtml = 19,
    Math = 20,
};
Q_ENUM_NS(ElementType)

enum ListType {
    OrderedList = 0,
    UnorderedList = 1,
    TaskList = 2
};
Q_ENUM_NS(ListType)

};

#endif // MDOPTIONS_H
