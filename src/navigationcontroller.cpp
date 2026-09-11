// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>

#include "navigationcontroller.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

using namespace Qt::StringLiterals;

NavigationController::NavigationController(QObject *parent)
    : QObject(parent)
{
}

bool NavigationController::mobileMode() const
{
    return m_mobileMode;
}

void NavigationController::setMobileMode(bool mobileMode)
{
    if (m_mobileMode == mobileMode) {
        return;
    }
    m_mobileMode = mobileMode;
    Q_EMIT mobileModeChanged();
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
    if (!m_notePath.isEmpty()) {
        m_notePath = QString{};
        Q_EMIT notePathChanged();
    }
    Q_EMIT notebookPathChanged();

    if (m_mobileMode) {
        return;
    }
    const QString dotDirectory = QDir::cleanPath(m_notebookPath + u'/' + u".directory"_s);

    if (QFile::exists(dotDirectory)) {
        const auto lastEntry = KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-LastEntry");
        if (lastEntry.length() > 0 && QFileInfo::exists(QDir::cleanPath(m_notebookPath + u'/' + lastEntry))) {
            setNotePath(lastEntry);
        }
    }
}

QString NavigationController::notePath() const
{
    return m_notePath;
}

QString NavigationController::absoluteNotePath() const
{
    if (m_notebookPath.isEmpty() || m_notePath.isEmpty()) {
        return {};
    }
    return QDir::cleanPath(m_notebookPath + QDir::separator() + m_notePath);
}

void NavigationController::setNotePath(const QString &notePath)
{
    if (m_notePath == notePath) {
        return;
    }
    m_notePath = notePath;
    Q_EMIT notePathChanged();

    if (m_notebookPath.isEmpty()) {
        return;
    }

    const QString dotDirectory = QDir::cleanPath(m_notebookPath + u'/' + u".directory"_s);
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(u"Desktop Entry"_s);
    if (notePath.isEmpty()) {
        desktopEntry.deleteEntry("X-MarkNote-LastEntry");
    } else {
        desktopEntry.writeEntry("X-MarkNote-LastEntry", notePath);
    }
    desktopFile.sync();
}

QString NavigationController::noteName() const
{
    return m_notePath.split(QLatin1Char('/')).last().replace(u".md"_s, QString{});
}

QUrl NavigationController::noteFullPath() const
{
    if (m_notebookPath.isEmpty() || m_notePath.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(m_notebookPath + u'/' + m_notePath);
}

bool NavigationController::sourceMode() const
{
    return m_sourceMode;
}

void NavigationController::setSourceMode(bool sourceMode)
{
    if (m_sourceMode != sourceMode) {
        m_sourceMode = sourceMode;
        Q_EMIT sourceModeChanged();
    }
}

bool NavigationController::openReplaceOnSourceMode() const
{
    return m_openReplaceOnSourceMode;
}

void NavigationController::setOpenReplaceOnSourceMode(bool open)
{
    if (m_openReplaceOnSourceMode != open) {
        m_openReplaceOnSourceMode = open;
        Q_EMIT openReplaceOnSourceModeChanged();
    }
}

QString NavigationController::initialSearchText() const
{
    return m_initialSearchText;
}

void NavigationController::setInitialSearchText(const QString &text)
{
    if (m_initialSearchText != text) {
        m_initialSearchText = text;
        Q_EMIT initialSearchTextChanged();
    }
}

#include "moc_navigationcontroller.cpp"
