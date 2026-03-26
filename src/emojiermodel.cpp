// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "emojiermodel.h"

void EmojierModel::constructEmojiVector(const Emoji *emoji_arr, size_t arr_size)
{
    m_shortcodes.clear();
    for (size_t i = 0; i < arr_size; i++) {
        m_shortcodes.push_back({QString::fromUtf8(emoji_arr[i].shortname), QString::fromUtf8(emoji_arr[i].code)});
    }
}

EmojierModel::EmojierModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QString locale = QLocale().name().left(2);

    const Emoji *emoji_arr = nullptr;
    size_t emoji_arr_size = 0;
    size_t en_emoji_arr_size = 0; // fallback

    for (Locale l : supported_locales) {
        if (strcmp(l.locale, "en") == 0) {
            en_emoji_arr_size = l.emoji_arr_size;
        }
        if (locale == QString::fromUtf8(l.locale)) {
            emoji_arr = l.emoji_arr;
            emoji_arr_size = l.emoji_arr_size;
        }
    }
    if (emoji_arr == nullptr) {
        // use english locale if unsupported locale is being used
        emoji_arr = emojis_en;
        emoji_arr_size = en_emoji_arr_size;
    }

    constructEmojiVector(emoji_arr, emoji_arr_size);
}

int EmojierModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_shortcodes.size());
}

QHash<int, QByteArray> EmojierModel::roleNames() const
{
    return {{ShortcodeRole, "shortcode"}, {EmojiCodeRole, "emojicode"}};
}

QVariant EmojierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_shortcodes.size())) {
        return {};
    }
    const EmojiItem &emoji = m_shortcodes[index.row()];

    if (role == ShortcodeRole) {
        return emoji.shortcode;
    } else if (role == EmojiCodeRole) {
        return emoji.code;
    }
    return {};
}