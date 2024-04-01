// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notebooksmodel.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QUrl>

#include <KConfigGroup>
#include <KDesktopFile>

NoteBooksModel::NoteBooksModel(const QString &_directory, QObject *parent)
    : QAbstractListModel(parent)
    , directory(_directory)
{
    directory.mkpath(QStringLiteral("."));
    qDebug() << directory.path();
}

NoteBooksModel::NoteBooksModel(QObject *parent)
    : NoteBooksModel(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + QStringLiteral("Notes"), parent)
{
}

int NoteBooksModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot).count();
}

QVariant NoteBooksModel::data(const QModelIndex &index, int role) const
{
    const auto entry = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());

    switch (role) {
    case Role::Path:
        return entry.filePath();

    case Role::Icon: {
        const QString dotDirectory = entry.filePath() % QDir::separator() % QStringLiteral(".directory");
        if (QFile::exists(dotDirectory)) {
            return KDesktopFile(dotDirectory).readIcon();
        } else {
            return QStringLiteral("addressbook-details");
        }
    }
    case Role::Color: {
        const QString dotDirectory = entry.filePath() % QDir::separator() % QStringLiteral(".directory");
        if (QFile::exists(dotDirectory)) {
            return KDesktopFile(dotDirectory).desktopGroup().readEntry("X-MarkNote-Color");
        } else {
            return QStringLiteral("#00000000");
        }
    }
    case Role::Name:
    case Qt::DisplayRole:
        return directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());
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
    beginResetModel();
    directory.mkdir(name);
    const QString dotDirectory = directory.path() % QDir::separator() % name % QDir::separator() % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();
    endResetModel();

    return directory.path() + u'/' + name;
}

void NoteBooksModel::editNoteBook(const QString &path, const QString &name, const QString &icon, const QString &color)
{
    const auto oldName = path.split(QLatin1Char('/')).constLast();

    const QString dotDirectory = directory.path() % QDir::separator() % oldName % QDir::separator() % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();

    if (oldName != name) {
        beginResetModel();
        QDir dir(directory.path());
        dir.rename(oldName, name);
        Q_EMIT noteBookRenamed(oldName, name, directory.path() + QDir::separator() + name);
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
    const auto entries = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    int i = 0;
    for (const auto &entry : entries) {
        if (entry.fileName() == dirName) {
            return index(i, 0);
        }
        i++;
    }

    return {};
}
