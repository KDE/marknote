// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef EMOJIERMODEL_H
#define EMOJIERMODEL_H

#include "emojis/emoji_shortnames.h"
#include <QAbstractListModel>
#include <QFile>
#include <QLocale>
#include <QObject>
#include <algorithm>

class EmojierModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit EmojierModel(QObject *parent = nullptr);

    enum Roles {
        ShortcodeRole = Qt::UserRole + 1,
        EmojiCodeRole,
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct EmojiItem {
        QString shortcode;
        QString code;
    };

    std::vector<EmojiItem> m_shortcodes;

    void constructEmojiVector(const Emoji *emoji_arr, size_t arr_size);
};

#endif // EMOJIERMODEL_H