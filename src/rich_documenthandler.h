// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef RICHDOCUMENTHANDLER_H
#define RICHDOCUMENTHANDLER_H

#include <QQuickItem>

#include "documenthandler.h"
#include "nestedlisthelper_p.h"
#include <QHash>

class QTextDocument;

class RichDocumentHandler : public DocumentHandler
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY underlineChanged)
    Q_PROPERTY(bool strikethrough READ strikethrough WRITE setStrikethrough NOTIFY strikethroughChanged)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY checkableChanged)

    Q_PROPERTY(bool canIndentList READ canIndentList NOTIFY cursorPositionChanged)
    Q_PROPERTY(bool canDedentList READ canDedentList NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentListStyle READ currentListStyle NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentHeadingLevel READ currentHeadingLevel NOTIFY cursorPositionChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)
    Q_PROPERTY(int searchMatchCount READ searchMatchCount NOTIFY searchMatchCountChanged)
    Q_PROPERTY(int searchCurrentMatch READ searchCurrentMatch NOTIFY searchCurrentMatchChanged)

public:
    explicit RichDocumentHandler(QObject *parent = nullptr);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool bold() const;
    void setBold(bool bold);

    bool italic() const;
    void setItalic(bool italic);

    bool underline() const;
    void setUnderline(bool underline);

    bool strikethrough() const;
    void setStrikethrough(bool strikethrough);

    bool checkable() const;
    void setCheckable(bool checkable);

    bool canIndentList() const;
    bool canDedentList() const;
    int currentListStyle() const;

    int currentHeadingLevel() const;

    Q_INVOKABLE QString currentLinkUrl() const;
    Q_INVOKABLE QString currentLinkText() const;

    Q_INVOKABLE void updateLink(const QString &linkUrl, const QString &linkText);
    Q_INVOKABLE void insertImage(const QUrl &imagePath);
    Q_INVOKABLE void insertTable(int rows, int columns);

    Q_INVOKABLE void pasteFromClipboard() override;

    Q_INVOKABLE void slotKeyPressed(int key);

public Q_SLOTS:
    void load(const QUrl &fileUrl) override;
    void saveAs(const QUrl &fileUrl) override;

    void indentListLess();
    void indentListMore();

    void setListStyle(int styleIndex);
    void setHeadingLevel(int level);

Q_SIGNALS:

    void alignmentChanged();

    void boldChanged();
    void italicChanged();
    void underlineChanged();
    void checkableChanged();
    void strikethroughChanged();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QString processImage(const QUrl &url);
    void reset() override;
    void selectLinkText(QTextCursor *cursor) const;
    QColor linkColor();
    bool evaluateReturnKeySupport(QKeyEvent *event);
    bool evaluateListSupport(QKeyEvent *event);
    bool handleShortcut(QKeyEvent *event);
    /// @returns whether the event should be handled by the normal TextEdit
    /// keyPressed event handler
    bool processKeyEvent(QKeyEvent *event);
    void moveLineUpDown(bool moveUp);
    void moveCursorBeginUpDown(bool moveUp);

    void regenerateColorScheme();

    QColor mLinkColor;

    /**c
     * The names of embedded images.
     * Used to easily obtain the names of the images.
     * New images are compared to the list and not added as resource if already present.
     */
    QStringList m_imageNames;
    QHash<QString, QString> m_imagePathLookup;

    NestedListHelper m_nestedListHelper;
    QString m_frontMatter;
    QString m_activeLink;

    bool m_lastBold;
    bool m_lastItalic;
    bool m_lastUnderline;
    bool m_lastStrikethrough;
};

#endif // RICHDOCUMENTHANDLER_H
