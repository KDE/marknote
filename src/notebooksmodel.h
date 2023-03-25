#ifndef NoteBooksModel_H
#define NoteBooksModel_H

#include <QAbstractListModel>
#include <QDir>

class NoteBooksModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        Path = Qt::UserRole + 1,
        Date,
        Name
    };
    Q_ENUM(Role)

    explicit NoteBooksModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &index) const override;

    QVariant data(const  QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addNoteBook(const QString &name);

    Q_INVOKABLE void deleteNoteBook(const QUrl &path);

    Q_INVOKABLE void renameNoteBook(const QUrl &path, const QString &name);

private:
    QDir directory;


};

#endif // NoteBooksModel_H
