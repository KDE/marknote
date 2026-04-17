// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "commandbarhelper.h"

#include <KirigamiActionCollection>

#include <QAction>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QSet>
#include <QTimer>

using namespace Qt::StringLiterals;

CommandBarHelper::CommandBarHelper(QObject *parent)
    : QObject(parent)
    , m_noteIcon(QIcon::fromTheme(u"document-edit-symbolic"_s))
{
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    m_refreshTimer->setInterval(300);
    connect(m_refreshTimer, &QTimer::timeout, this, &CommandBarHelper::updateNoteActions);

    m_storageWatcher = new QFileSystemWatcher(this);
    connect(m_storageWatcher, &QFileSystemWatcher::directoryChanged, this, &CommandBarHelper::scheduleRefresh);
}

CommandBarHelper::~CommandBarHelper() = default;

AbstractKirigamiApplication *CommandBarHelper::application() const
{
    return m_application;
}

void CommandBarHelper::setApplication(AbstractKirigamiApplication *app)
{
    if (m_application == app) {
        return;
    }

    m_application = app;
    Q_EMIT applicationChanged();

    scheduleRefresh();
}

QString CommandBarHelper::storagePath() const
{
    return m_storagePath;
}

void CommandBarHelper::setStoragePath(const QString &path)
{
    const QString cleanPath = QDir::cleanPath(path);
    if (m_storagePath == cleanPath) {
        return;
    }

    m_storagePath = cleanPath;
    Q_EMIT storagePathChanged();

    rebuildWatchPaths();
    scheduleRefresh();
}

void CommandBarHelper::refreshNoteActions()
{
    scheduleRefresh();
}

QString CommandBarHelper::actionNameForNote(const QString &notebookPath, const QString &noteFileName)
{
    const QByteArray key = (notebookPath + u'/' + noteFileName).toUtf8();
    const QByteArray digest = QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex();
    return u"dynamic_note_"_s + QString::fromLatin1(digest);
}

void CommandBarHelper::removeStaleActions(const QSet<QString> &previousNames, const QSet<QString> &currentNames)
{
    KirigamiActionCollection *actionCollection = m_application ? m_application->mainCollection() : nullptr;
    if (!actionCollection) {
        return;
    }

    for (const QString &staleActionName : previousNames - currentNames) {
        if (QAction *staleAction = actionCollection->action(staleActionName)) {
            actionCollection->removeAction(staleAction);
            if (staleAction->parent() == this) {
                staleAction->deleteLater();
            }
        }
    }
}

void CommandBarHelper::updateNoteActions()
{
    if (!m_application) {
        return;
    }

    KirigamiActionCollection *actionCollection = m_application->mainCollection();
    if (!actionCollection) {
        return;
    }

    const QSet<QString> previousActionNames = m_dynamicActionNames;
    QSet<QString> currentActionNames;

    if (m_storagePath.isEmpty()) {
        removeStaleActions(previousActionNames, currentActionNames);
        m_dynamicActionNames.clear();
        return;
    }

    QDir storageDir(m_storagePath);
    if (!storageDir.exists()) {
        removeStaleActions(previousActionNames, currentActionNames);
        m_dynamicActionNames.clear();
        return;
    }

    const auto notebooks = storageDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    int actionIndex = 0;
    constexpr int maxResults = 200;

    for (const QFileInfo &notebook : notebooks) {
        if (actionIndex >= maxResults) {
            break;
        }

        const QString notebookPath = notebook.filePath();
        const QString notebookName = notebook.fileName();

        QDir notebookDir(notebookPath);
        const auto notes = notebookDir.entryInfoList({u"*.md"_s}, QDir::Files, QDir::Name);

        for (const QFileInfo &note : notes) {
            if (actionIndex >= maxResults) {
                break;
            }

            const QString noteName = note.completeBaseName();
            QString actionText = noteName;

            if (!notebookName.isEmpty()) {
                actionText += u" · "_s + notebookName;
            }

            const QString actionName = actionNameForNote(notebookPath, note.fileName());
            currentActionNames.insert(actionName);

            if (auto *existingAction = actionCollection->action(actionName)) {
                existingAction->setText(actionText);
                existingAction->setIcon(m_noteIcon);
                existingAction->setVisible(true);
                existingAction->setEnabled(true);
                actionIndex++;
                continue;
            }

            auto *action = new QAction(m_noteIcon, actionText, this);

            const QString capturedNotebookPath = notebookPath;
            const QString capturedNotePath = note.fileName();
            connect(
                action,
                &QAction::triggered,
                this,
                [this, capturedNotebookPath, capturedNotePath]() {
                    Q_EMIT navigateToNote(capturedNotebookPath, capturedNotePath);
                },
                Qt::QueuedConnection);

            actionCollection->addAction(actionName, action);
            actionIndex++;
        }
    }

    removeStaleActions(previousActionNames, currentActionNames);
    m_dynamicActionNames = currentActionNames;
    rebuildWatchPaths();
}

void CommandBarHelper::scheduleRefresh()
{
    if (!m_refreshTimer) {
        return;
    }

    m_refreshTimer->stop();
    m_refreshTimer->start();
}

void CommandBarHelper::rebuildWatchPaths()
{
    if (!m_storageWatcher) {
        return;
    }

    if (!m_storageWatcher->directories().isEmpty()) {
        m_storageWatcher->removePaths(m_storageWatcher->directories());
    }

    if (m_storagePath.isEmpty()) {
        return;
    }

    QDir storageDir(m_storagePath);
    if (!storageDir.exists()) {
        return;
    }

    QStringList watchDirs;
    watchDirs.append(storageDir.absolutePath());

    const auto notebooks = storageDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &notebook : notebooks) {
        watchDirs.append(notebook.absoluteFilePath());
    }

    m_storageWatcher->addPaths(watchDirs);
}

#include "moc_commandbarhelper.cpp"
