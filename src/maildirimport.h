// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class MaildirImport : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MaildirImport(QObject *parent = nullptr);

    /// Import and convert maildir entries to a specific destionationDir
    Q_INVOKABLE void import(const QUrl &maildir, const QUrl &destinationDir);

Q_SIGNALS:
    void entryConverted(const QString &title);
    void errorOccurred(const QString &errorMessage);
};