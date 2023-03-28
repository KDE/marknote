#include "notebooksmodel.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QStringBuilder>

#include <KDesktopFile>
#include <KConfigGroup>

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
    switch (role) {
    case Role::Path:
        return directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row()).filePath();

    case Role::Icon: {
        const QString dotDirectory = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row()).filePath() % QDir::separator() % ".directory";
        if (QFile::exists(dotDirectory)) {
            return KDesktopFile(dotDirectory).readIcon();
        } else {
            return QStringLiteral("addressbook-details");
        }
    }
    case Role::Color: {
        const QString dotDirectory = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot).at(index.row()).filePath() % QDir::separator() % ".directory";
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
        {Role::Color, "color"}
    };
}

void NoteBooksModel::addNoteBook(const QString &name, const QString &icon, const QString &color)
{
    qDebug() << Q_FUNC_INFO;

    beginResetModel();
    directory.mkdir(name);
    const QString dotDirectory = directory.path() % QDir::separator() % name % QDir::separator() % ".directory";
    KConfig desktopFile(dotDirectory, KConfig::SimpleConfig);
    auto desktopEntry = desktopFile.group("Desktop Entry");
    desktopEntry.writeEntry("Icon", icon);
    desktopEntry.writeEntry("X-MarkNote-Color", color);
    desktopFile.sync();
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
