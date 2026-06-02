// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ASYNCDOCBUILDER_H
#define ASYNCDOCBUILDER_H

#include <QFuture>
#include <QObject>
#include <QPromise>
#include <md4qt/parser.h>

class AsyncDocBuilder : public QObject
{
    Q_OBJECT

public:
    using DocPointer = QSharedPointer<MD::Document>;

public:
    explicit AsyncDocBuilder(QObject *parent = nullptr);
    ~AsyncDocBuilder();

    void loadDocument(const QString &filePath);

Q_SIGNALS:
    void documentReady(const DocPointer &document);

private:
    static void parseDocument(QPromise<DocPointer> &promise, const QString &filePath);
};

#endif // ASYNCDOCBUILDER_H
