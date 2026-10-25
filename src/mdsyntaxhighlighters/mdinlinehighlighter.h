// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDINLINEHIGHLIGHTER_H
#define MDINLINEHIGHLIGHTER_H

#include "markdowntheme.h"
#include "mdinlineparser/mdinlineparser.h"

#include <QColor>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QSyntaxHighlighter>

class MDInlineHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor dimmedColor READ dimmedColor WRITE setDimmedColor NOTIFY dimmedColorChanged)
    Q_PROPERTY(QColor codeColor READ codeColor WRITE setCodeColor NOTIFY codeColorChanged)
    Q_PROPERTY(QString fixedFontFamily READ fixedFontFamily WRITE setFixedFontFamily NOTIFY fixedFontFamilyChanged)

public:
    explicit MDInlineHighlighter(QObject *parent = nullptr);
    ~MDInlineHighlighter() override = default;

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    bool active() const;
    void setActive(bool active);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    QColor dimmedColor() const;
    void setDimmedColor(const QColor &color);

    QColor codeColor() const;
    void setCodeColor(const QColor &color);

    QString fixedFontFamily() const;
    void setFixedFontFamily(const QString &family);

protected:
    void highlightBlock(const QString &text) override;

Q_SIGNALS:
    void documentChanged();
    void activeChanged();
    void textColorChanged();
    void dimmedColorChanged();
    void codeColorChanged();
    void fixedFontFamilyChanged();

private:
    void updateSpans();
    void applyFormat(int start, int length, const QTextCharFormat &fmt);

    QQuickTextDocument *m_quickDocument = nullptr;
    bool m_active = true;
    bool m_dirty = true;
    QString m_lastParsedText;

    MarkdownTheme m_theme;
    MDInlineStyleParser m_parser;
    QList<MDInlineSpan> m_cachedSpans;
};

#endif // MDINLINEHIGHLIGHTER_H
