#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QDir>
class NotesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        Path = Qt::UserRole + 1,
        Date,
        Name
    };
    Q_ENUM(Role)

    explicit NotesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &index) const override;

    QVariant data(const  QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNote(const QString &name);

    Q_INVOKABLE void deleteNote(const QUrl &path);

    Q_INVOKABLE void renameNote(const QUrl &path, const QString &name);


signals:

private:
    QDir directory;


};

#endif // NOTESMODEL_H
