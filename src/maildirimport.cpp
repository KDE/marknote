// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#include "maildirimport.h"

#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include <KLocalizedString>
#include <KMime/Message>

using namespace Qt::StringLiterals;

MaildirImport::MaildirImport(QObject *parent)
    : QObject()
{
}

void MaildirImport::import(const QUrl &maildir, const QUrl &destinationDir)
{
    QFileInfo maildirInfo(maildir.toLocalFile());
    Q_ASSERT(maildirInfo.exists());
    Q_ASSERT(maildirInfo.isDir());

    QFileInfo destinationInfo(destinationDir.toLocalFile());
    Q_ASSERT(destinationInfo.exists());
    Q_ASSERT(destinationInfo.isDir());

    const QStringList subdirs{u"/tmp"_s, u"/cur"_s, u"/new"_s};
    for (const auto &subdir : subdirs) {
        QDir dir(maildirInfo.canonicalFilePath() + subdir);
        if (dir.exists()) {
            const auto entries = dir.entryInfoList(QDir::Files);
            for (const auto &entry : entries) {
                QFile mimeFile(entry.canonicalFilePath());
                if (!mimeFile.open(QIODevice::ReadOnly)) {
                    continue;
                }

                const auto mailData = KMime::CRLFtoLF(mimeFile.readAll());
                auto msg = KMime::Message::Ptr(new KMime::Message);
                msg->setContent(mailData);
                msg->parse();

                const auto subject = msg->subject()->asUnicodeString();
                const auto content = msg->decodedContent();

                QFile markdownFile(destinationDir.toLocalFile() + u'/' + QString::fromUtf8(QFile::encodeName(subject)) + u".md"_s);
                if (!markdownFile.open(QIODevice::WriteOnly)) {
                    qWarning() << "Not writable" << markdownFile.fileName() << markdownFile.errorString();
                    Q_EMIT errorOccurred(i18nc("@status", "An error occurred while writing to '%1'", markdownFile.fileName()));
                    continue;
                }
                markdownFile.write(content);

                Q_EMIT entryConverted(subject);
            }
        }
    }
}
