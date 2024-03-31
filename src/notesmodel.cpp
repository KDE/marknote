// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notesmodel.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QUrl>

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NotesModel::rowCount(const QModelIndex &index) const
{
    return m_path.isEmpty() ? 0 : directory.entryList(QDir::Files).count();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Role::Path:
        return QUrl::fromLocalFile(directory.entryInfoList(QDir::Files).at(index.row()).filePath());
    case Role::Date:
        return directory.entryInfoList(QDir::Files).at(index.row()).lastModified(QTimeZone::LocalTime);
    case Role::Name:
        return directory.entryInfoList(QDir::Files).at(index.row()).fileName().replace(QStringLiteral(".md"), QString());
    }

    Q_UNREACHABLE();

    return {};
}

QHash<int, QByteArray> NotesModel::roleNames() const
{
    return {{Role::Date, "date"}, {Role::Path, "path"}, {Role::Name, "name"}};
}

QString NotesModel::addNote(const QString &name)
{
    beginResetModel();
    const QString path = m_path + QDir::separator() + name + QStringLiteral(".md");
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write("# " + name.toUtf8());
    } else {
        qDebug() << "Failed to create file at" << m_path;
    }
    endResetModel();
    return path;
}

void NotesModel::deleteNote(const QUrl &path)
{
    beginResetModel();
    QFile::remove(path.toLocalFile());
    endResetModel();
}

void NotesModel::renameNote(const QUrl &path, const QString &name)
{
    QString newPath = directory.path() + QDir::separator() + name + QStringLiteral(".md");
    beginResetModel();
    QFile::rename(path.toLocalFile(), newPath);
    endResetModel();
}

QString NotesModel::path() const
{
    return m_path;
}

void NotesModel::setPath(const QString &newPath)
{
    if (m_path == newPath)
        return;

    beginResetModel();
    m_path = newPath;
    directory = QDir(m_path);
    endResetModel();
    Q_EMIT pathChanged();
}
