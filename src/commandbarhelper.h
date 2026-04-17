// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <AbstractKirigamiApplication>
#include <QIcon>
#include <QObject>
#include <QQmlEngine>

class QAction;
class QFileSystemWatcher;
class QTimer;
class AbstractKirigamiApplication;

class CommandBarHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString storagePath READ storagePath WRITE setStoragePath NOTIFY storagePathChanged)
    Q_PROPERTY(AbstractKirigamiApplication *application READ application WRITE setApplication NOTIFY applicationChanged)

public:
    explicit CommandBarHelper(QObject *parent = nullptr);
    ~CommandBarHelper() override;

    AbstractKirigamiApplication *application() const;
    void setApplication(AbstractKirigamiApplication *app);

    QString storagePath() const;
    void setStoragePath(const QString &path);

    Q_INVOKABLE void refreshNoteActions();

Q_SIGNALS:
    void storagePathChanged();
    void applicationChanged();
    void navigateToNote(const QString &notebookPath, const QString &notePath);

private:
    static QString actionNameForNote(const QString &notebookPath, const QString &noteFileName);
    void updateNoteActions();
    void removeStaleActions(const QSet<QString> &previousNames, const QSet<QString> &currentNames);
    void scheduleRefresh();
    void rebuildWatchPaths();

    AbstractKirigamiApplication *m_application = nullptr;
    QString m_storagePath;
    QSet<QString> m_dynamicActionNames;
    QTimer *m_refreshTimer = nullptr;
    QFileSystemWatcher *m_storageWatcher = nullptr;
    QIcon m_noteIcon;
};
