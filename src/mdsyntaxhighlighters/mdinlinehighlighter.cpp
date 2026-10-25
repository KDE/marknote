// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdinlinehighlighter.h"

#include <QTextDocument>

MDInlineHighlighter::MDInlineHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
}

QQuickTextDocument *MDInlineHighlighter::document() const
{
    return m_quickDocument;
}

void MDInlineHighlighter::setDocument(QQuickTextDocument *document)
{
    if (m_quickDocument == document) {
        return;
    }

    if (m_quickDocument && m_quickDocument->textDocument()) {
        disconnect(m_quickDocument->textDocument(), nullptr, this, nullptr);
    }

    m_quickDocument = document;
    if (m_quickDocument && m_quickDocument->textDocument()) {
        QSyntaxHighlighter::setDocument(m_quickDocument->textDocument());
        connect(m_quickDocument->textDocument(), &QTextDocument::contentsChanged, this, [this]() {
            m_dirty = true;
        });
    } else {
        QSyntaxHighlighter::setDocument(nullptr);
    }

    m_dirty = true;
    m_lastParsedText.clear();
    m_cachedSpans.clear();

    if (m_active && m_quickDocument) {
        rehighlight();
    }
    Q_EMIT documentChanged();
}

bool MDInlineHighlighter::active() const
{
    return m_active;
}

void MDInlineHighlighter::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    if (m_active) {
        m_dirty = true;
        rehighlight();
    }
    Q_EMIT activeChanged();
}

QColor MDInlineHighlighter::textColor() const
{
    return m_theme.textColor();
}

void MDInlineHighlighter::setTextColor(const QColor &color)
{
    if (m_theme.textColor() == color) {
        return;
    }

    m_theme.setTextColor(color);
    m_dirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT textColorChanged();
}

QColor MDInlineHighlighter::dimmedColor() const
{
    return m_theme.dimmedColor();
}

void MDInlineHighlighter::setDimmedColor(const QColor &color)
{
    if (m_theme.dimmedColor() == color) {
        return;
    }

    m_theme.setDimmedColor(color);
    m_dirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT dimmedColorChanged();
}

QColor MDInlineHighlighter::codeColor() const
{
    return m_theme.codeColor();
}

void MDInlineHighlighter::setCodeColor(const QColor &color)
{
    if (m_theme.codeColor() == color) {
        return;
    }

    m_theme.setCodeColor(color);
    m_dirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT codeColorChanged();
}

QString MDInlineHighlighter::fixedFontFamily() const
{
    return m_theme.fixedFontFamily();
}

void MDInlineHighlighter::setFixedFontFamily(const QString &family)
{
    if (m_theme.fixedFontFamily() == family) {
        return;
    }

    m_theme.setFixedFontFamily(family);
    m_dirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT fixedFontFamilyChanged();
}

void MDInlineHighlighter::updateSpans()
{
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        m_cachedSpans.clear();
        m_lastParsedText.clear();
        return;
    }

    const QString text = m_quickDocument->textDocument()->toPlainText();
    if (text == m_lastParsedText && !m_dirty) {
        return;
    }

    m_cachedSpans = m_parser.parse(text);
    m_lastParsedText = text;
    m_dirty = false;
}

void MDInlineHighlighter::highlightBlock(const QString &text)
{
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        return;
    }

    updateSpans();

    const int textLen = text.length();
    if (textLen <= 0) {
        return;
    }

    const int blockStart = currentBlock().position();
    const int blockEnd = blockStart + textLen;
    const QTextCharFormat delimFmt = m_theme.delimiterFormat();

    for (const auto &span : m_cachedSpans) {
        if (span.start >= blockEnd || span.end < blockStart) {
            continue;
        }

        const QTextCharFormat styleFmt = m_theme.formatForInline(span.type);

        const int openStart = qMax(blockStart, span.start);
        const int openEnd = qMin(blockEnd, span.start + span.openDelimLen);
        if (openStart < openEnd) {
            setFormat(openStart - blockStart, openEnd - openStart, delimFmt);
        }

        const int textStart = qMax(blockStart, span.start + span.openDelimLen);
        const int textEnd = qMin(blockEnd, span.end - span.closeDelimLen + 1);
        if (textStart < textEnd) {
            applyFormat(textStart - blockStart, textEnd - textStart, styleFmt);
        }

        const int closeStart = qMax(blockStart, span.end - span.closeDelimLen + 1);
        const int closeEnd = qMin(blockEnd, span.end + 1);
        if (closeStart < closeEnd) {
            setFormat(closeStart - blockStart, closeEnd - closeStart, delimFmt);
        }
    }
}

void MDInlineHighlighter::applyFormat(int start, int length, const QTextCharFormat &fmt)
{
    for (int i = 0; i < length; ++i) {
        QTextCharFormat current = format(start + i);
        current.merge(fmt);
        setFormat(start + i, 1, current);
    }
}
