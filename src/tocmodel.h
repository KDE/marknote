// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QPointer>
#include <QQuickTextDocument>

class TocModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged REQUIRED)

public:
    enum Role {
        Title = Qt::UserRole + 1,
        Level,
        CursorPosition,
    };
    Q_ENUM(Role)

    explicit TocModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

Q_SIGNALS:
    void documentChanged();

private:
    void setTextDocument(QTextDocument *document);
    void updateModel();

    QPointer<QQuickTextDocument> m_document;
    QPointer<QTextDocument> m_textDocument;
    struct Entry {
        QString title;
        int level;
        int position;
    };
    QList<Entry> m_entries;
};
