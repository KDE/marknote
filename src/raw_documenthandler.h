// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef RAWDOCUMENTHANDLER_H
#define RAWDOCUMENTHANDLER_H

#include "documenthandler.h"

class RawDocumentHandler : public DocumentHandler
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

public:
    explicit RawDocumentHandler(QObject *parent = nullptr);

    Q_INVOKABLE void pasteFromClipboard() override;

public Q_SLOTS:
    void load(const QUrl &fileUrl) override;
    void saveAs(const QUrl &fileUrl) override;

    // Q_SIGNALS:
    // any additional signals can be defined here

private:
    void reset() override;
};

#endif