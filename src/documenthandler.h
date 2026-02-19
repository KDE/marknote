// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QQuickItem>

#include "nestedlisthelper_p.h"
#include <QFont>
#include <QHash>
#include <QObject>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QUrl>

class QTextDocument;

class DocumentHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QQuickItem *textArea READ textArea WRITE setTextArea NOTIFY textAreaChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
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

    // Q_PROPERTY(bool list READ list WRITE setList NOTIFY listChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)
    Q_PROPERTY(int searchMatchCount READ searchMatchCount NOTIFY searchMatchCountChanged)
    Q_PROPERTY(int searchCurrentMatch READ searchCurrentMatch NOTIFY searchCurrentMatchChanged)

public:
    explicit DocumentHandler(QObject *parent = nullptr);

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    QQuickItem *textArea() const;
    void setTextArea(QQuickItem *textArea);

    int cursorPosition() const;
    void setCursorPosition(int position);

    int selectionStart() const;
    void setSelectionStart(int position);

    int selectionEnd() const;
    void setSelectionEnd(int position);

    QString fontFamily() const;
    void setFontFamily(const QString &family);

    QColor textColor() const;
    void setTextColor(const QColor &color);

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

    // bool list() const;
    // void setList(bool list);

    int fontSize() const;
    void setFontSize(int size);

    QString fileName() const;
    QString fileType() const;
    QUrl fileUrl() const;

    bool modified() const;
    void setModified(bool m);

    int searchMatchCount() const;
    int searchCurrentMatch() const;

    Q_INVOKABLE QString currentLinkUrl() const;
    Q_INVOKABLE QString currentLinkText() const;
    Q_INVOKABLE [[nodiscard]] QString anchorAt(const QPointF &p) const;
    Q_INVOKABLE void updateLink(const QString &linkUrl, const QString &linkText);
    Q_INVOKABLE void insertImage(const QUrl &imagePath);
    Q_INVOKABLE void insertTable(int rows, int columns);
    Q_INVOKABLE int findText(const QString &searchTerm);
    Q_INVOKABLE void findNext();
    Q_INVOKABLE void findPrevious();
    Q_INVOKABLE void clearSearch();

    Q_INVOKABLE void copyWholeNote();
    Q_INVOKABLE void pasteFromClipboard();

    Q_INVOKABLE void slotKeyPressed(int key);
    Q_INVOKABLE void slotMouseMovedWithControl(QPointF position);
    Q_INVOKABLE void slotMouseMovedWithControlReleased();

public Q_SLOTS:
    void load(const QUrl &fileUrl);
    void saveAs(const QUrl &fileUrl);

    void indentListLess();
    void indentListMore();

    void setListStyle(int styleIndex);
    void setHeadingLevel(int level);

Q_SIGNALS:
    void documentChanged();
    void textAreaChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();

    void fontFamilyChanged();
    void textColorChanged();
    void alignmentChanged();

    void boldChanged();
    void italicChanged();
    void underlineChanged();
    void checkableChanged();
    void strikethroughChanged();

    // void listChanged();

    void fontSizeChanged();

    void textChanged();
    void fileUrlChanged();

    void loaded(const QString &text, int format);
    void error(const QString &message);

    void modifiedChanged();
    void searchMatchCountChanged();
    void searchCurrentMatchChanged();

    void focusUp();
    void focusDown();
    void copy();
    void cut();
    void undo();
    void redo();
    void moveCursor(int position);
    void selectCursor(int start, int end);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QString processImage(const QUrl &url);
    void reset();
    QTextCursor textCursor() const;
    void selectLinkText(QTextCursor *cursor) const;
    QTextDocument *textDocument() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    QColor linkColor();
    bool evaluateReturnKeySupport(QKeyEvent *event);
    bool evaluateListSupport(QKeyEvent *event);
    bool handleShortcut(QKeyEvent *event);
    /// @returns whether the event should be handled by the normal TextEdit
    /// keyPressed event handler
    bool processKeyEvent(QKeyEvent *event);
    void moveLineUpDown(bool moveUp);
    void moveCursorBeginUpDown(bool moveUp);

    void deleteWordBack();
    void deleteWordForward();

    void regenerateColorScheme();

    QQuickTextDocument *m_document;
    QQuickItem *m_textArea;
    QColor mLinkColor;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    /**
     * The names of embedded images.
     * Used to easily obtain the names of the images.
     * New images are compared to the list and not added as resource if already present.
     */
    QStringList m_imageNames;
    QHash<QString, QString> m_imagePathLookup;

    QFont m_font;
    QUrl m_fileUrl;
    NestedListHelper m_nestedListHelper;
    QString m_frontMatter;
    QString m_activeLink;
    QString m_searchTerm;
    int m_searchCurrentMatch = -1;
    QList<QTextCursor> m_searchMatches;

    // Cache data to dismiss redundant UI calls.
    QString m_lastFontFamily;
    Qt::Alignment m_lastAlignment;
    bool m_lastBold;
    bool m_lastItalic;
    bool m_lastUnderline;
    bool m_lastStrikethrough;
    int m_lastFontSize;
    QColor m_lastTextColor;
};

#endif // DOCUMENTHANDLER_H
