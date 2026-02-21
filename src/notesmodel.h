// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QFileSystemWatcher>
#include <QQmlEngine>

class NotesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged REQUIRED)

public:
    enum Role {
        Path = Qt::UserRole + 1,
        FileUrl,
        Date,
        Month,
        Name,
        Color,
    };
    Q_ENUM(Role)

    explicit NotesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    /// \return the path of the newly added not
    Q_INVOKABLE QString addNote(const QString &name);

    Q_INVOKABLE void deleteNote(const QUrl &path);

    Q_INVOKABLE void renameNote(const QUrl &path, const QString &name);

    Q_INVOKABLE void duplicateNote(const QUrl &path);

    /// Export a note to HTML.
    /// \param path The path of the note to export.
    /// \param destination The destination of the note to export.
    Q_INVOKABLE void exportToHtml(const QUrl &path, const QUrl &destination);

    /// Export a note to PDF.
    /// \param path The path of the note to export.
    /// \param destination The destination of the note to export.
    Q_INVOKABLE void exportToPdf(const QUrl &path, const QUrl &destination);

    /// Export a note to ODT.
    /// \param path The path of the note to export.
    /// \param destination The destination of the note to export.
    Q_INVOKABLE void exportToOdt(const QUrl &path, const QUrl &destination);

    QString path() const;
    void setPath(const QString &newPath);

Q_SIGNALS:
    void pathChanged();
    void errorOccured(const QString &errorMessage);

private:
    void updateColor();
    void updateEntries();

    QFileInfoList m_entries;
    QString m_path;
    QString m_color;
    QFileSystemWatcher m_watcher;
};

#endif // NOTESMODEL_H
