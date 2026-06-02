// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "asyncdocbuilder.h"

#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

AsyncDocBuilder::AsyncDocBuilder(QObject *parent)
    : QObject(parent)
{
}

AsyncDocBuilder::~AsyncDocBuilder() = default;

void AsyncDocBuilder::loadDocument(const QString &filePath)
{
    QFutureWatcher<DocPointer> *watcher = new QFutureWatcher<DocPointer>(this);

    connect(watcher, &QFutureWatcher<DocPointer>::finished, this, [this, watcher]() {
        Q_EMIT documentReady(watcher->result());
        watcher->deleteLater();
    });

    QFuture<DocPointer> future = QtConcurrent::run(&AsyncDocBuilder::parseDocument, filePath);
    watcher->setFuture(future);
}

void AsyncDocBuilder::parseDocument(QPromise<DocPointer> &promise, const QString &filePath)
{
    MD::Parser parser;

    auto doc = parser.parse(filePath, false);

    promise.addResult(doc);
}
