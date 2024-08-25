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
    : QObject(parent)
{
}

static QString cleanFileName(const QString &name)
{
    QString fileName = name.trimmed();

    // We need to replace colons with underscores since those cause problems with
    // KFileDialog (bug in KFileDialog though) and also on Windows filesystems.
    // We also look at the special case of ": ", since converting that to "_ "
    // would look strange, simply "_" looks better.
    // https://issues.kolab.org/issue3805
    fileName.replace(QLatin1StringView(": "), QStringLiteral("_"));
    // replace all ':' with '_' because ':' isn't allowed on FAT volumes
    fileName.replace(QLatin1Char(':'), QLatin1Char('_'));
    // better not use a dir-delimiter in a filename
    fileName.replace(QLatin1Char('/'), QLatin1Char('_'));
    fileName.replace(QLatin1Char('\\'), QLatin1Char('_'));

#ifdef Q_OS_WINDOWS
    // replace all '.' with '_', not just at the start of the filename
    // but don't replace the last '.' before the file extension.
    int i = fileName.lastIndexOf(QLatin1Char('.'));
    if (i != -1) {
        i = fileName.lastIndexOf(QLatin1Char('.'), i - 1);
    }

    while (i != -1) {
        fileName.replace(i, 1, QLatin1Char('_'));
        i = fileName.lastIndexOf(QLatin1Char('.'), i - 1);
    }
#endif

    // replace all '~' with '_', not just leading '~' either.
    fileName.replace(QLatin1Char('~'), QLatin1Char('_'));

    return fileName;
}

void MaildirImport::import(const QUrl &maildir, const QUrl &destinationDir)
{
    QFileInfo maildirInfo(maildir.toLocalFile());
    if (!maildirInfo.exists()) {
        qWarning() << maildir.toLocalFile() << " doesn't exist";
        return;
    }
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

                QFile markdownFile(destinationDir.toLocalFile() + u'/' + cleanFileName(subject) + u".md"_s);
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

#include "moc_maildirimport.cpp"
