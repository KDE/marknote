// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MARKNOTE_DOCUMENTHANDLER_H
#define MARKNOTE_DOCUMENTHANDLER_H

#include <QFont>
#include <QObject>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QTextCursor>
#include <QUrl>

class QTextDocument;

class DocumentHandler : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QQuickItem *textArea READ textArea WRITE setTextArea NOTIFY textAreaChanged)
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

    QQuickTextDocument *m_document;
    QQuickItem *m_textArea;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    QFont m_font;
    QUrl m_fileUrl;

    QString m_searchTerm;
    int m_searchCurrentMatch = -1;
    QList<QTextCursor> m_searchMatches;

    QString m_lastFontFamily;
    int m_lastFontSize;
    Qt::Alignment m_lastAlignment;

    QColor m_lastTextColor;
};

#endif // MARKNOTE_DOCUMENTHANDLER_H