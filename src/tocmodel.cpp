// SPDX-FileCopyrightText: 2026 Shubham Shinde <shubshinde8381@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "tocmodel.h"

#include <QTextBlock>
#include <QTextDocument>

TocModel::TocModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TocModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.count();
}

QVariant TocModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.count()) {
        return {};
    }

    const auto &entry = m_entries.at(index.row());
    switch (role) {
    case Role::Title:
        return entry.title;
    case Role::Level:
        return entry.level;
    case Role::CursorPosition:
        return entry.position;
    }

    return {};
}

QHash<int, QByteArray> TocModel::roleNames() const
{
    return {
        {Role::Title, "title"},
        {Role::Level, "level"},
        {Role::CursorPosition, "cursorPosition"},
    };
}

QQuickTextDocument *TocModel::document() const
{
    return m_document;
}

void TocModel::setDocument(QQuickTextDocument *document)
{
    if (m_document == document) {
        return;
    }

    m_document = document;

    if (m_document && m_document->textDocument()) {
        setTextDocument(m_document->textDocument());
    } else {
        setTextDocument(nullptr);
    }

    Q_EMIT documentChanged();
}

void TocModel::setTextDocument(QTextDocument *document)
{
    if (m_textDocument == document) {
        return;
    }

    if (m_textDocument) {
        disconnect(m_textDocument, &QTextDocument::contentsChanged, this, &TocModel::updateModel);
    }

    m_textDocument = document;

    if (m_textDocument) {
        connect(m_textDocument, &QTextDocument::contentsChanged, this, &TocModel::updateModel);
        updateModel();
    } else {
        beginResetModel();
        m_entries.clear();
        endResetModel();
    }
}

void TocModel::updateModel()
{
    if (!m_textDocument) {
        beginResetModel();
        m_entries.clear();
        endResetModel();
        return;
    }

    QList<Entry> newEntries;
    QTextDocument *doc = m_textDocument;

    for (QTextBlock it = doc->begin(); it.isValid(); it = it.next()) {
        const int level = it.blockFormat().headingLevel();
        if (level > 0) {
            newEntries.append({it.text(), level, it.position()});
        }
    }

    bool changed = (m_entries.count() != newEntries.count());
    if (!changed) {
        for (int i = 0; i < m_entries.count(); ++i) {
            if (m_entries[i].title != newEntries[i].title || m_entries[i].level != newEntries[i].level || m_entries[i].position != newEntries[i].position) {
                changed = true;
                break;
            }
        }
    }

    if (changed) {
        beginResetModel();
        m_entries = newEntries;
        endResetModel();
    }
}
