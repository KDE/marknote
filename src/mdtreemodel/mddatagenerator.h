// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDDATAGENERATOR_H
#define MDDATAGENERATOR_H

#include <QSharedPointer>
#include <QVariantMap>
#include <md4qt/doc.h>

namespace MDDataGenerator
{
void applyIntermediateData(QSharedPointer<MD::Document> to, QSharedPointer<MD::Document> from);

QVariantMap fromHeading(const QSharedPointer<MD::Item> &item, QSharedPointer<MD::Document> fullDoc);
QVariantMap fromParagraph(const QSharedPointer<MD::Item> &item, QSharedPointer<MD::Document> fullDoc);
QVariantMap fromList(const QSharedPointer<MD::Item> &item);
QVariantMap fromListItem(const QSharedPointer<MD::Item> &item);
QVariantMap fromCodeBlock(const QSharedPointer<MD::Item> &item);
QVariantMap fromBlockquote(const QSharedPointer<MD::Item> &item);
QVariantMap fromPageBreak(const QSharedPointer<MD::Item> &item);
QVariantMap fromAnchor(const QSharedPointer<MD::Item> &item);
QVariantMap fromHorizontalLine(const QSharedPointer<MD::Item> &item);
QVariantMap fromFootnote(const QSharedPointer<MD::Item> &item);
QVariantMap fromTable(const QSharedPointer<MD::Item> &item, QSharedPointer<MD::Document> fullDoc);

QString toHtml(const QSharedPointer<MD::Item> &item, QSharedPointer<MD::Document> fullDoc = {});
QString toMd(const QSharedPointer<MD::Item> &item);
}

#endif // MDDATAGENERATOR_H
