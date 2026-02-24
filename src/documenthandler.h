// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

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
    QML_ANONYMOUS

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)

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

    int fontSize() const;
    void setFontSize(int size);

    QString fileName() const;
    QString fileType() const;
    QUrl fileUrl() const;

    bool modified() const;
    void setModified(bool m);

    int searchMatchCount() const;
    int searchCurrentMatch() const;

    Q_INVOKABLE [[nodiscard]] QString anchorAt(const QPointF &p) const;
    Q_INVOKABLE void clearUndoRedoStacks();

    Q_INVOKABLE int findText(const QString &searchTerm);
    Q_INVOKABLE void findNext();
    Q_INVOKABLE void findPrevious();
    Q_INVOKABLE void clearSearch();

    virtual Q_INVOKABLE void pasteFromClipboard() = 0;

    Q_INVOKABLE void slotMouseMovedWithControl(QPointF position);
    Q_INVOKABLE void slotMouseMovedWithControlReleased();

public Q_SLOTS:
    virtual void load(const QUrl &fileUrl) = 0;
    virtual void saveAs(const QUrl &fileUrl) = 0;

Q_SIGNALS:
    void documentChanged();
    void textAreaChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();

    void fontFamilyChanged();
    void textColorChanged();
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
    virtual void reset() = 0;
    QTextCursor textCursor() const;
    QTextDocument *textDocument() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    void deleteWordBack();
    void deleteWordForward();

    [[nodiscard]] bool isCodeBlock(const QTextBlock &block) const;

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
    int m_lastFontSize;
    QColor m_lastTextColor;
};
