// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDINLINEPARSER_H
#define MDINLINEPARSER_H

#include "mdoptions/mdoptions.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

struct MDInlineSpan {
    Q_GADGET
    Q_PROPERTY(int start MEMBER start CONSTANT)
    Q_PROPERTY(int end MEMBER end CONSTANT)
    Q_PROPERTY(MDOptions::InlineStyle type MEMBER type CONSTANT)
    Q_PROPERTY(int openDelimLen MEMBER openDelimLen CONSTANT)
    Q_PROPERTY(int closeDelimLen MEMBER closeDelimLen CONSTANT)

public:
    int start = -1;
    int end = -1;
    MDOptions::InlineStyle type = MDOptions::InlineStyle::None;
    int openDelimLen = 0;
    int closeDelimLen = 0;
};
Q_DECLARE_METATYPE(MDInlineSpan)

class MDInlineStyleParser : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit MDInlineStyleParser(QObject *parent = nullptr);

    Q_INVOKABLE QList<MDInlineSpan> parse(const QString &text);
};

#endif // MDINLINEPARSER_H
