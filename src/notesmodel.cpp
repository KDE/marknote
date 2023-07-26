// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "notesmodel.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QUrl>

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NotesModel::rowCount(const QModelIndex &index) const
{
    return m_path.isEmpty()? 0 : directory.entryList(QDir::Files).count();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Role::Path:
        return QUrl::fromLocalFile(directory.entryInfoList(QDir::Files).at(index.row()).filePath());
    case Role::Date:
        return directory.entryInfoList(QDir::Files).at(index.row()).birthTime();
    case Role::Name:
        return directory.entryInfoList(QDir::Files).at(index.row()).fileName().replace(".md", "");
    }

    Q_UNREACHABLE();

    return {};
}

QHash<int, QByteArray> NotesModel::roleNames() const
{
    return {
        {Role::Date, "date"},
        {Role::Path, "path"},
        {Role::Name, "name"}
    };
}

void NotesModel::addNote(const QString &name)
{
    beginResetModel();
    QFile file(m_path + QDir::separator() + name + ".md");
    if(file.open(QFile::WriteOnly)){
        file.write("");
    } else {
        qDebug() << "Failed to create file at" << m_path;
    }
    endResetModel();
}

void NotesModel::deleteNote(const QUrl &path)
{
    beginResetModel();
    QFile::remove(path.toLocalFile());
    endResetModel();

}

void NotesModel::renameNote(const QUrl &path, const QString &name)
{
    QString newPath = directory.path() + QDir::separator() + name + ".md";
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
