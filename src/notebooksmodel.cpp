// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notebooksmodel.h"
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDesktopFile>

NoteBooksModel::NoteBooksModel(QObject *parent)
    : QAbstractListModel(parent)
{
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
    };
}

QString NoteBooksModel::addNoteBook(const QString &name, const QString &icon, const QString &color)
{
    Q_ASSERT(m_directory);

    beginResetModel();
    m_directory->mkdir(name);
    const QString dotDirectory = m_directory->path() % u'/' % name % u'/' % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();
    endResetModel();

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
        beginResetModel();
        QDir dir(m_directory->path());
        dir.rename(oldName, name);
        Q_EMIT noteBookRenamed(oldName, name, m_directory->path() + u'/' + name);
        endResetModel();
        return;
    }

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
    QDir directory(path);
    // TODO(carl): Move to trash instead
    directory.removeRecursively();
    endRemoveRows();
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
    Q_EMIT storagePathChanged();
}

#include "moc_notebooksmodel.cpp"
