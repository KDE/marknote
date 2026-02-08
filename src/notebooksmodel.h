// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef NoteBooksModel_H
#define NoteBooksModel_H

#include <QAbstractListModel>
#include <QDir>
#include <QFileSystemWatcher>
#include <QQmlEngine>

class NoteBooksModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString storagePath READ storagePath WRITE setStoragePath NOTIFY storagePathChanged)

public:
    enum Role {
        Path = Qt::UserRole + 1,
        Name,
        Icon,
        Color,
        NoteCount,
    };
    Q_ENUM(Role)

    explicit NoteBooksModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString storagePath() const;
    void setStoragePath(const QString &storagePath);

    Q_INVOKABLE QString addNoteBook(const QString &name, const QString &icon, const QString &color);
    Q_INVOKABLE void editNoteBook(const QString &path, const QString &name, const QString &icon, const QString &color);
    Q_INVOKABLE void deleteNoteBook(const QString &path);

    Q_INVOKABLE QString iconNameForPath(const QString &path) const;
    Q_INVOKABLE QString colorForPath(const QString &path) const;

Q_SIGNALS:
    void noteBookRenamed(const QString &oldName, const QString &newName, const QString &path);
    void storagePathChanged();

private:
    QModelIndex indexForPath(const QString &path) const;
    std::optional<QDir> m_directory;
    QFileSystemWatcher m_watcher;
    QString m_storagePath;
    void updateWatches();
};

#endif // NoteBooksModel_H
