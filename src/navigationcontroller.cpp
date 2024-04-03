// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#include "navigationcontroller.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>

#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace Qt::StringLiterals;

NavigationController::NavigationController(QObject *parent)
    : QObject(parent)
{
}

QString NavigationController::notebookPath() const
{
    return m_notebookPath;
}

QString NavigationController::notebookName() const
{
    return m_notebookPath.split(QLatin1Char('/')).constLast();
}

void NavigationController::setNotebookPath(const QString &notebookPath)
{
    if (m_notebookPath == notebookPath) {
        return;
    }
    m_notebookPath = notebookPath;
    Q_EMIT notebookPathChanged();

    const QString dotDirectory = m_notebookPath + u'/' + QStringLiteral(".directory");

    if (QFile::exists(dotDirectory)) {
        const auto lastEntry = KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-LastEntry");
        if (lastEntry.length() > 0 && QFileInfo::exists(m_notebookPath + u'/' + lastEntry)) {
            setNotePath(lastEntry);
        } else {
            setNotePath(QString{});
        }
    }
}

QString NavigationController::notePath() const
{
    return m_notePath;
}

void NavigationController::setNotePath(const QString &notePath)
{
    QString path = notePath;
    if (notePath.isEmpty()) {
        QDir dir(m_notebookPath);
        const auto entries = dir.entryInfoList(QDir::Files);
        if (entries.isEmpty()) {
            path = QString();
        } else {
            path = dir.entryInfoList(QDir::Files).at(0).fileName();
        }
    }

    if (m_notePath == path) {
        return;
    }
    m_notePath = path;
    Q_EMIT notePathChanged();

    const QString dotDirectory = m_notebookPath + u'/' + QStringLiteral(".directory");

    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("X-MarkNote-LastEntry", path);
    desktopFile.sync();
}

QString NavigationController::noteName() const
{
    return m_notePath.split(QLatin1Char('/')).last().replace(u".md"_s, QString{});
}

QUrl NavigationController::noteFullPath() const
{
    return QUrl::fromLocalFile(m_notebookPath + u'/' + m_notePath);
}

#include "moc_navigationcontroller.cpp"
