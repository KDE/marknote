// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2015-2024 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#include "rich_documenthandler.h"
#include "asyncdocbuilder/asyncdocbuilder.h"
#include "mdtreemodel/treeitem.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

using namespace Qt::StringLiterals;

RichDocumentHandler::RichDocumentHandler(QObject *parent)
    : DocumentHandler(parent)
    , m_mdTreeModel(nullptr)
    , m_popupVisible(false)
{
    m_document = nullptr;
    m_textArea = nullptr;
    m_cursorPosition = -1;
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

QString RichDocumentHandler::currentEmojicode() const
{
    return m_currentEmojicode;
}

void RichDocumentHandler::setCurrentEmojicode(const QString &text)
{
    if (m_currentEmojicode != text) {
        m_currentEmojicode = text;
        Q_EMIT currentEmojicodeChanged();
    }
}

bool RichDocumentHandler::popupVisible() const
{
    return m_popupVisible;
}

void RichDocumentHandler::setPopupVisible(bool status)
{
    if (m_popupVisible != status) {
        m_popupVisible = status;
        Q_EMIT popupVisibleChanged();
    }
}

void RichDocumentHandler::checkForShortcode(const QString &text, int cursorPosition)
{
    const QString left_text = text.left(cursorPosition);
    if (left_text.isEmpty()) {
        setPopupVisible(false);
        return;
    }
    int colon_posn = -1;

    for (int i = left_text.length() - 1; i >= 0; i--) {
        static const QRegularExpression regex(QStringLiteral("^[\\w:-]$"), QRegularExpression::UseUnicodePropertiesOption);
        QRegularExpressionMatch match = regex.match(left_text[i]);

        if (!match.hasMatch()) {
            break;
        }
        if (left_text[i] == u':') {
            colon_posn = i;
            break;
        }
    }

    if (colon_posn == -1) {
        setPopupVisible(false);
        return;
    }

    QString shortcode = left_text.right(left_text.length() - colon_posn - 1);
    if (shortcode.isEmpty()) {
        setPopupVisible(false);
        return;
    }
    setCurrentEmojicode(shortcode);
    setPopupVisible(true);
}

void RichDocumentHandler::load(const QUrl &fileUrl)
{
    clearSearch();

    if (fileUrl == m_fileUrl)
        return;

    m_fileUrl = fileUrl;
    Q_EMIT fileUrlChanged();

    if (!QFile::exists(fileUrl.toLocalFile()))
        return;

    if (m_models.contains(fileUrl)) {
        m_mdTreeModel = m_models.value(fileUrl);
        Q_EMIT treeModelChanged();
    } else {
        m_mdTreeModel = new MDTreeModel(this);
        m_models.insert(fileUrl, m_mdTreeModel);
        Q_EMIT treeModelChanged();

        AsyncDocBuilder *builder = new AsyncDocBuilder(this);
        builder->loadDocument(fileUrl.toLocalFile());

        connect(
            builder,
            &AsyncDocBuilder::documentReady,
            this,
            [this, fileUrl](const AsyncDocBuilder::DocPointer &doc) {
                if (MDTreeModel *model = m_models.value(fileUrl)) {
                    model->setDocument(doc);
                }
            },
            Qt::SingleShotConnection);
    }
}

void RichDocumentHandler::saveAs(const QUrl &fileUrl)
{
    const QUrl targetUrl = fileUrl.isEmpty() ? m_fileUrl : fileUrl;
    MDTreeModel *treeModelToSave = m_models.value(targetUrl, m_mdTreeModel);

    if (treeModelToSave && !targetUrl.isEmpty()) {
        if (treeModelToSave->saveToFile(targetUrl)) {
            if (targetUrl != m_fileUrl) {
                m_fileUrl = targetUrl;
                Q_EMIT fileUrlChanged();
            }
            return;
        }
    }
}

MDTreeModel *RichDocumentHandler::treeModel() const
{
    return m_mdTreeModel;
}

int RichDocumentHandler::searchMatchCount() const
{
    return m_richSearchMatches.size();
}

int RichDocumentHandler::searchCurrentMatch() const
{
    return m_richSearchCurrentMatch;
}

TreeItem *RichDocumentHandler::searchMatchedBlock() const
{
    if (m_richSearchCurrentMatch >= 0 && m_richSearchCurrentMatch < m_richSearchMatches.size()) {
        return m_richSearchMatches.at(m_richSearchCurrentMatch).block;
    }
    return nullptr;
}

void RichDocumentHandler::collectMatches(TreeItem *item, int topLevelIndex, const QString &searchTerm, QList<RichSearchMatch> &matches)
{
    if (!item) {
        return;
    }

    const MDOptions::ElementType type = item->type();

    if (type == MDOptions::ElementType::Paragraph || type == MDOptions::ElementType::Heading) {
        const QVariantMap map = item->data();
        const QString text = map.value(u"md"_s).toString();
        if (!text.isEmpty()) {
            qsizetype pos = 0;
            while ((pos = text.indexOf(searchTerm, pos, Qt::CaseInsensitive)) != -1) {
                matches.append(RichSearchMatch{item, topLevelIndex, static_cast<int>(pos), static_cast<int>(searchTerm.length()), -1, -1});
                pos += searchTerm.length();
            }
        }
    } else if (type == MDOptions::ElementType::Code) {
        const QVariantMap map = item->data();
        const QString text = map.value(u"text"_s).toString();
        if (!text.isEmpty()) {
            qsizetype pos = 0;
            while ((pos = text.indexOf(searchTerm, pos, Qt::CaseInsensitive)) != -1) {
                matches.append(RichSearchMatch{item, topLevelIndex, static_cast<int>(pos), static_cast<int>(searchTerm.length()), -1, -1});
                pos += searchTerm.length();
            }
        }
    } else if (type == MDOptions::ElementType::Table) {
        const QVariantMap map = item->data();
        if (map.contains(u"mdData"_s)) {
            const QList<QVariantList> mdData = map[u"mdData"_s].value<QList<QVariantList>>();
            for (int r = 0; r < mdData.size(); ++r) {
                for (int c = 0; c < mdData[r].size(); ++c) {
                    const QString cellText = mdData[r][c].toString();
                    if (!cellText.isEmpty()) {
                        qsizetype pos = 0;
                        while ((pos = cellText.indexOf(searchTerm, pos, Qt::CaseInsensitive)) != -1) {
                            matches.append(RichSearchMatch{item, topLevelIndex, static_cast<int>(pos), static_cast<int>(searchTerm.length()), r, c});
                            pos += searchTerm.length();
                        }
                    }
                }
            }
        }
    }

    const int count = item->childCount();
    for (int i = 0; i < count; ++i) {
        collectMatches(item->child(i), topLevelIndex, searchTerm, matches);
    }
}

void RichDocumentHandler::updateCurrentMatchNavigation()
{
    TreeItem *matchedBlock = searchMatchedBlock();
    if (m_mdTreeModel) {
        m_mdTreeModel->setSearchMatchedBlock(matchedBlock);
    }
    if (m_richSearchCurrentMatch >= 0 && m_richSearchCurrentMatch < m_richSearchMatches.size()) {
        const int topLevel = m_richSearchMatches.at(m_richSearchCurrentMatch).topLevelBlockIndex;
        Q_EMIT requestScrollToBlock(topLevel);
    }
}

int RichDocumentHandler::findText(const QString &searchTerm)
{
    m_richSearchMatches.clear();
    m_richSearchCurrentMatch = -1;

    const QString cleanTerm = searchTerm.trimmed();
    if (cleanTerm.isEmpty() || !m_mdTreeModel || !m_mdTreeModel->rootItem()) {
        if (m_mdTreeModel) {
            m_mdTreeModel->setSearchMatchedBlock(nullptr);
        }
        Q_EMIT searchMatchCountChanged();
        Q_EMIT searchCurrentMatchChanged();
        return 0;
    }

    TreeItem *root = m_mdTreeModel->rootItem();
    const int topLevelCount = root->childCount();
    for (int i = 0; i < topLevelCount; ++i) {
        collectMatches(root->child(i), i, cleanTerm, m_richSearchMatches);
    }

    if (!m_richSearchMatches.isEmpty()) {
        m_richSearchCurrentMatch = 0;
    }

    updateCurrentMatchNavigation();

    Q_EMIT searchMatchCountChanged();
    Q_EMIT searchCurrentMatchChanged();

    return m_richSearchMatches.size();
}

void RichDocumentHandler::findNext()
{
    if (m_richSearchMatches.isEmpty()) {
        return;
    }

    m_richSearchCurrentMatch = (m_richSearchCurrentMatch + 1) % m_richSearchMatches.size();
    updateCurrentMatchNavigation();
    Q_EMIT searchCurrentMatchChanged();
}

void RichDocumentHandler::findPrevious()
{
    if (m_richSearchMatches.isEmpty()) {
        return;
    }

    m_richSearchCurrentMatch = (m_richSearchCurrentMatch - 1 + m_richSearchMatches.size()) % m_richSearchMatches.size();
    updateCurrentMatchNavigation();
    Q_EMIT searchCurrentMatchChanged();
}

void RichDocumentHandler::clearSearch()
{
    m_richSearchMatches.clear();
    m_richSearchCurrentMatch = -1;
    if (m_mdTreeModel) {
        m_mdTreeModel->setSearchMatchedBlock(nullptr);
    }
    Q_EMIT searchMatchCountChanged();
    Q_EMIT searchCurrentMatchChanged();
}

#include "moc_rich_documenthandler.cpp"
