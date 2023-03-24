#include "notesmodel.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QUrl>

NotesModel::NotesModel(QObject *parent)
    : QAbstractListModel(parent)
    , directory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "Notes")
{
    directory.mkpath(".");
    qDebug() << directory.path();
}

int NotesModel::rowCount(const QModelIndex &index) const
{
    return directory.entryList(QDir::Files).count();
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
    QFile file(directory.path() + QDir::separator() + name + ".md");
    if(file.open(QFile::WriteOnly)){
        file.write("");
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
