#include "notebooksmodel.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QUrl>

NoteBooksModel::NoteBooksModel(QObject *parent)
    : QAbstractListModel(parent)
    , directory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "Notes")
{
    directory.mkpath(".");
    qDebug() << directory.path();
}

int NoteBooksModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot).count();
}

QVariant NoteBooksModel::data(const QModelIndex &index, int role) const
{
    qDebug() << "Data";
    switch (role) {
    case Role::Path:
        return directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row()).filePath();

    case Role::Date:
        return "";

    case Role::Name:
        return directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row());
    }

    Q_UNREACHABLE();

    return {};
}

QHash<int, QByteArray> NoteBooksModel::roleNames() const
{
    return {
        {Role::Date, "date"},
        {Role::Path, "path"},
        {Role::Name, "name"}
    };
}

void NoteBooksModel::addNoteBook(const QString &name)
{
    qDebug() << Q_FUNC_INFO;

    beginResetModel();
    directory.mkdir(name);
    endResetModel();
}

void NoteBooksModel::deleteNoteBook(const QString &name)
{
    beginResetModel();
    QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "Notes" + QDir::separator() + name).removeRecursively();
    endResetModel();

}

void NoteBooksModel::renameNoteBook(const QUrl &path, const QString &name)
{
    QString newPath = directory.path() + QDir::separator() + name + ".md";
    beginResetModel();
    QFile::rename(path.toLocalFile(), newPath);
    endResetModel();

}
