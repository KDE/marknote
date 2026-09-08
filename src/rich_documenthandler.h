// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RICHDOCUMENTHANDLER_H
#define RICHDOCUMENTHANDLER_H

#include "documenthandler.h"
#include "mdtreemodel/mdtreemodel.h"
#include <QHash>
#include <QString>

class RichDocumentHandler : public DocumentHandler
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(MDTreeModel *treeModel READ treeModel NOTIFY treeModelChanged)
    Q_PROPERTY(TreeItem *searchMatchedBlock READ searchMatchedBlock NOTIFY searchCurrentMatchChanged)
    Q_PROPERTY(QString currentEmojicode READ currentEmojicode WRITE setCurrentEmojicode NOTIFY currentEmojicodeChanged)
    Q_PROPERTY(bool popupVisible READ popupVisible WRITE setPopupVisible NOTIFY popupVisibleChanged)

public:
    explicit RichDocumentHandler(QObject *parent = nullptr);

    MDTreeModel *treeModel() const;

    QString currentEmojicode() const;
    void setCurrentEmojicode(const QString &text);

    bool popupVisible() const;
    void setPopupVisible(bool status);

    Q_INVOKABLE void checkForShortcode(const QString &text, int cursorPosition);

    int searchMatchCount() const override;
    int searchCurrentMatch() const override;
    TreeItem *searchMatchedBlock() const;

    Q_INVOKABLE int findText(const QString &searchTerm) override;
    Q_INVOKABLE void findNext() override;
    Q_INVOKABLE void findPrevious() override;
    Q_INVOKABLE void clearSearch() override;

public Q_SLOTS:
    void load(const QUrl &fileUrl) override;
    void saveAs(const QUrl &fileUrl) override;

Q_SIGNALS:
    void treeModelChanged();
    void requestScrollToBlock(int topLevelBlockIndex);
    void currentEmojicodeChanged();
    void popupVisibleChanged();

private:
    void reset() override
    {
    }

    struct RichSearchMatch {
        TreeItem *block = nullptr;
        int topLevelBlockIndex = -1;
        int startPos = 0;
        int length = 0;
        int tableRow = -1;
        int tableCol = -1;
    };

    void collectMatches(TreeItem *item, int topLevelIndex, const QString &searchTerm, QList<RichSearchMatch> &matches);
    void updateCurrentMatchNavigation();

    QHash<QUrl, MDTreeModel *> m_models;
    MDTreeModel *m_mdTreeModel;
    QList<RichSearchMatch> m_richSearchMatches;
    int m_richSearchCurrentMatch = -1;

    QString m_currentEmojicode;
    bool m_popupVisible = false;
};

#endif // RICHDOCUMENTHANDLER_H
