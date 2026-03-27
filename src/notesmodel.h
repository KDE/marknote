// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QDir>
#include <QFileSystemModel>
#include <QIdentityProxyModel>
#include <QQmlEngine>

class NotesModel : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QModelIndex rootIndex READ rootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged REQUIRED)

public:
    enum Role {
        FileUrl = Qt::UserRole + 100, // Shifted to prevent collisions with QFileSystemModel roles
        Path,
        Date,
        Month,
        Name,
        Color,
    };
    Q_ENUM(Role)

    explicit NotesModel(QObject *parent = nullptr);

    /**
     * @brief Intercepts requests for a source model index.
     * Maps the proxy's absolute root (invalid index) to the internal
     * file system index representing the current notebook path.
     * @param proxyIndex The index from the proxy model.
     * @return The corresponding index in the underlying QFileSystemModel.
     */
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    /**
     * @brief Translates a source model index back to a proxy index.
     * Ensures that the internal notebook root folder is hidden from
     * the view, appearing as the absolute root of the tree.
     * @param sourceIndex The index from the QFileSystemModel.
     * @return The translated index for the proxy model.
     */
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    /**
     * @brief Returns the number of items in the current branch.
     * Prevents the UI from attempting to query or draw system drives
     * before the notebook folder has been fully initialized.
     * @param parent The parent index being queried.
     * @return The count of files or subfolders available.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Checks if a specific model item contains children.
     * Used by the tree view to determine if an item (folder) can be expanded.
     * @param parent The index to check for children.
     * @return True if the item is a directory with visible contents.
     */
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Provides the root index for the QML view.
     * Because of the proxy mapping, this returns an invalid index to signal
     * that the notebook path is the top-level item.
     * @return An invalid QModelIndex representing the tree root.
     */
    Q_INVOKABLE QModelIndex rootIndex() const;

    /**
     * @brief Triggers a scan of a subfolder's contents.
     * Explicitly commands the underlying QFileSystemModel to fetch data
     * for folders that have not yet been indexed by the OS.
     * @param path The absolute local path to the folder to fetch.
     */
    Q_INVOKABLE void fetchMore(const QString &path);

    Q_INVOKABLE QString addNote(const QString &name);
    Q_INVOKABLE void deleteNote(const QUrl &path);
    Q_INVOKABLE void renameNote(const QUrl &path, const QString &name);
    Q_INVOKABLE void duplicateNote(const QUrl &path);
    Q_INVOKABLE void copyWholeNote(const QUrl &path);

    /**
     * @brief Handles moving a note or folder to a new destination.
     * Validates the operation to prevent moving items into themselves
     * or into non-existent directories.
     * @param source The URL of the file or folder to move.
     * @param destination The URL of the target directory.
     * @return True if the move operation was successful.
     */
    Q_INVOKABLE bool moveEntry(const QUrl &source, const QUrl &destination);

    Q_INVOKABLE bool exportToHtml(const QUrl &path, const QUrl &destination);
    Q_INVOKABLE bool exportToPdf(const QUrl &path, const QUrl &destination);
    Q_INVOKABLE bool exportToOdt(const QUrl &path, const QUrl &destination);

    Q_INVOKABLE bool noteExists(const QString &noteName) const;

    QString path() const;

    /**
     * @brief Sets the active notebook directory and initializes monitoring.
     * Normalizes the path and resets the internal root index to point
     * to the new local file system location.
     * @param newPath The local path to the notebook folder.
     */
    void setPath(const QString &newPath);

Q_SIGNALS:
    void pathChanged();
    void rootIndexChanged();
    void errorOccurred(const QString &errorMessage);

private:
    void updateColor();

    QFileSystemModel *m_fsModel;
    QModelIndex m_rootIndex;
    QString m_path;
    QString m_color;
};

#endif // NOTESMODEL_H
