// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDHIGHLIGHTER_H
#define MDHIGHLIGHTER_H

#include "markdowntheme.h"

#include <md4qt/doc.h>

#include <QColor>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QSharedPointer>
#include <QSyntaxHighlighter>

struct FormatRange {
    int start = 0;
    int length = 0;
    QTextCharFormat format;
};

class MDHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Mode {
        Full = 0,
        Inline
    };
    Q_ENUM(Mode)

    enum RehighlightMode {
        FullRehighlight = 0,
        RehighlightOnlyVisible
    };
    Q_ENUM(RehighlightMode)

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(RehighlightMode rehighlightMode READ rehighlightMode WRITE setRehighlightMode NOTIFY rehighlightModeChanged)
    Q_PROPERTY(QString fixedFontFamily READ fixedFontFamily WRITE setFixedFontFamily NOTIFY fixedFontFamilyChanged)
    Q_PROPERTY(qreal baseFontSize READ baseFontSize WRITE setBaseFontSize NOTIFY baseFontSizeChanged)

public:
    explicit MDHighlighter(QObject *parent = nullptr);
    ~MDHighlighter() override = default;

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    bool active() const;
    void setActive(bool active);

    Mode mode() const;
    void setMode(Mode mode);

    RehighlightMode rehighlightMode() const;
    void setRehighlightMode(RehighlightMode mode);

    QString fixedFontFamily() const;
    void setFixedFontFamily(const QString &family);

    qreal baseFontSize() const;
    void setBaseFontSize(qreal size);

public Q_SLOTS:
    void doRehighlight();
    void updateViewportInfo(int contentY, int height);
    void rehighlightVisible();

protected:
    void highlightBlock(const QString &text) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void documentChanged();
    void activeChanged();
    void modeChanged();
    void rehighlightModeChanged();
    void fixedFontFamilyChanged();
    void baseFontSizeChanged();

private:
    void parseDoc();
    void updateBlockFormats();
    void applyFormat(int start, int length, const QTextCharFormat &fmt);
    void initTheme();
    int firstVisibleBlockNumber() const;

    QQuickTextDocument *m_quickDocument = nullptr;
    bool m_active = true;
    Mode m_mode = Mode::Full;
    RehighlightMode m_rehighlightMode = FullRehighlight;
    qreal m_baseFontSize = 10.0;
    int m_contentY;
    int m_contentHeight;

    MarkdownTheme m_theme;
    QList<QList<FormatRange>> m_blockFormats;
    QSharedPointer<MD::Document> m_doc;
    QList<int> m_lineOffsets;
};

#endif // MDHIGHLIGHTER_H
