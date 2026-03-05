// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notebooksmodel.h"
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDesktopFile>
#include <QFileSystemWatcher>

NoteBooksModel::NoteBooksModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &path) {
        const auto idx = indexForPath(path);
        if (idx.isValid()) {
            Q_EMIT dataChanged(idx, idx, {Role::NoteCount});
        }
    });
}

int NoteBooksModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() || !m_directory ? 0 : m_directory->entryList(QDir::AllDirs | QDir::NoDotAndDotDot).count();
}

QVariant NoteBooksModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(m_directory);

    const auto entry = m_directory->entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());

    switch (role) {
    case Role::Path:
        return entry.filePath();

    case Role::Icon: {
        const QString dotDirectory = entry.filePath() % u'/' % QStringLiteral(".directory");
        if (QFile::exists(dotDirectory)) {
            return KDesktopFile(dotDirectory).readIcon();
        } else {
            return QStringLiteral("addressbook-details");
        }
    }
    case Role::Color: {
        const QString dotDirectory = entry.filePath() % u'/' % QStringLiteral(".directory");
        if (QFile::exists(dotDirectory)) {
            return KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-Color");
        } else {
            return QStringLiteral("#00000000");
        }
    }
    case Role::NoteCount: {
        const QDir dir(entry.filePath());
        const auto entries = dir.entryList(QStringList() << QStringLiteral("*.md"), QDir::Files);
        return entries.count();
    }
    case Role::Name:
    case Qt::DisplayRole:
        return m_directory->entryList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());
    }

    return {};
}

QHash<int, QByteArray> NoteBooksModel::roleNames() const
{
    return {
        {Role::Icon, "iconName"},
        {Role::Path, "path"},
        {Role::Name, "name"},
        {Role::Color, "color"},
        {Role::NoteCount, "noteCount"},
    };
}

QString NoteBooksModel::addNoteBook(const QString &name, const QString &icon, const QString &color)
{
    Q_ASSERT(m_directory);

    // Predict the exact row index where QDir will place the new directory
    const QStringList currentDirs = m_directory->entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    int insertRow = currentDirs.count();

    for (int i = 0; i < currentDirs.count(); ++i) {
        // QDir sorts by name case-insensitively by default
        if (QString::compare(name, currentDirs.at(i), Qt::CaseInsensitive) < 0) {
            insertRow = i;
            break;
        }
    }

    // Notify the QML view that a single row is about to be inserted
    beginInsertRows(QModelIndex(), insertRow, insertRow);

    m_directory->mkdir(name);
    m_watcher.addPath(m_directory->path() % u'/' % name);
    const QString dotDirectory = m_directory->path() % u'/' % name % u'/' % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();

    endInsertRows();

    return m_directory->path() + u'/' + name;
}

void NoteBooksModel::editNoteBook(const QString &path, const QString &name, const QString &icon, const QString &color)
{
    Q_ASSERT(m_directory);

    const auto oldName = path.split(QLatin1Char('/')).constLast();

    const QString dotDirectory = m_directory->path() % u'/' % oldName % u'/' % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();

    if (oldName != name) {
        // Find the current index before renaming
        const QStringList currentDirs = m_directory->entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        int oldIndex = currentDirs.indexOf(oldName);

        if (oldIndex >= 0) {
            // Simulate the list without the old directory to find the new alphabetical index
            QStringList simulatedDirs = currentDirs;
            simulatedDirs.removeAt(oldIndex);

            int newIndex = simulatedDirs.count();
            for (int i = 0; i < simulatedDirs.count(); ++i) {
                if (QString::compare(name, simulatedDirs.at(i), Qt::CaseInsensitive) < 0) {
                    newIndex = i;
                    break;
                }
            }

            QDir dir(m_directory->path());

            // If the alphabetical order changes, move the row
            if (oldIndex != newIndex) {
                // Qt quirk: when moving items down the list, the destination row must be offset by +1
                int destIndex = newIndex > oldIndex ? newIndex + 1 : newIndex;

                beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), destIndex);
                dir.rename(oldName, name);
                endMoveRows();
            } else {
                dir.rename(oldName, name);
            }

            // Update the file watcher to track the new directory name instead of the old one
            m_watcher.removePath(path);
            m_watcher.addPath(m_directory->path() % u'/' % name);

            // Notify QML that the item's data (its name/path) has updated
            const QModelIndex changedIdx = index(newIndex, 0);
            Q_EMIT dataChanged(changedIdx, changedIdx);
        }

        Q_EMIT noteBookRenamed(oldName, name, m_directory->path() + u'/' + name);
        return;
    }

    // Only the icon or color changed, no rename needed
    const auto idx = indexForPath(path);
    if (idx.isValid()) {
        Q_EMIT dataChanged(idx, idx);
    }
}

void NoteBooksModel::deleteNoteBook(const QString &path)
{
    const auto idx = indexForPath(path);
    if (!idx.isValid()) {
        return;
    }

    beginRemoveRows({}, idx.row(), idx.row());

    m_watcher.removePath(path);

    QDir directory(path);
    // TODO(carl): Move to trash instead
    directory.removeRecursively();
    endRemoveRows();
}

void NoteBooksModel::moveNote(const QString &noteUri, const QString &notebookPath)
{
    const QString notePath = QUrl(noteUri).toLocalFile();

    if (notePath.isEmpty()) {
        qWarning() << "Invalid note URI:" << noteUri;
        return;
    }

    if (!QFile::exists(notePath)) {
        qWarning() << "Source note does not exist:" << notePath;
        return;
    }

    const QString fileName = QFileInfo(notePath).fileName();
    const QString newPath = QDir(notebookPath).filePath(fileName);

    if (QFile::exists(newPath)) {
        qWarning() << "Target note already exists:" << newPath;
        return;
    }

    if (!QFile::rename(notePath, newPath)) {
        qWarning() << "Failed to move note from" << notePath << "to" << newPath;
    }
}

QString NoteBooksModel::iconNameForPath(const QString &path) const
{
    const auto idx = indexForPath(path);
    if (idx.isValid()) {
        return data(idx, Role::Icon).toString();
    }
    return QStringLiteral("addressbook-details");
}

QString NoteBooksModel::colorForPath(const QString &path) const
{
    const auto idx = indexForPath(path);
    if (idx.isValid()) {
        return data(idx, Role::Color).toString();
    }
    return QStringLiteral("#ffffff");
}

QModelIndex NoteBooksModel::indexForPath(const QString &path) const
{
    const auto dirName = path.split(QLatin1Char('/')).constLast();
    const auto entries = m_directory->entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    int i = 0;
    for (const auto &entry : entries) {
        if (entry.fileName() == dirName) {
            return index(i, 0);
        }
        i++;
    }

    return {};
}

QString NoteBooksModel::storagePath() const
{
    return m_storagePath;
}

void NoteBooksModel::setStoragePath(const QString &storagePath)
{
    if (m_storagePath == storagePath) {
        return;
    }
    m_storagePath = storagePath;
    beginResetModel();
    m_directory = QDir(m_storagePath);
    m_directory->mkpath(QStringLiteral("."));
    endResetModel();
    updateWatches();
    Q_EMIT storagePathChanged();
}

void NoteBooksModel::updateWatches()
{
    const QStringList files = m_watcher.files();
    if (!files.isEmpty()) {
        m_watcher.removePaths(files);
    }

    const QStringList dirs = m_watcher.directories();
    if (!dirs.isEmpty()) {
        m_watcher.removePaths(dirs);
    }

    if (!m_directory) {
        return;
    }

    const auto entries = m_directory->entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const auto &entry : entries) {
        m_watcher.addPath(entry.filePath());
    }
}

#include "moc_notebooksmodel.cpp"
