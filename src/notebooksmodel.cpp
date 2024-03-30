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

NoteBooksModel::NoteBooksModel(QObject *parent)
    : QAbstractListModel(parent)
    , directory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + QStringLiteral("Notes"))
{
    directory.mkpath(QStringLiteral("."));
    qDebug() << directory.path();
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
            return QStringLiteral("addressbook-details");
        }
    }
    case Role::Name:
        return directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());
    }

    Q_UNREACHABLE();

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

void NoteBooksModel::addNoteBook(const QString &name, const QString &icon, const QString &color)
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
}

void NoteBooksModel::editNoteBook(int row, const QString &name, const QString &icon, const QString &color)
{
    const auto oldName = data(index(row, 0), Role::Name).toString();

    const QString dotDirectory = directory.path() % QDir::separator() % oldName % QDir::separator() % QStringLiteral(".directory");
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group(QStringLiteral("Desktop Entry"));
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();

    if (oldName != name) {
        QDir dir(directory.path());
        dir.rename(oldName, name);
        Q_EMIT noteBookRenamed(oldName, name, row);
    }

    Q_EMIT dataChanged(index(row, 0), index(row, 0));
}

void NoteBooksModel::deleteNoteBook(const QString &name)
{
    beginResetModel();
    QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + QStringLiteral("Notes") + QDir::separator() + name)
        .removeRecursively();
    endResetModel();
}
