// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdhighlighter.h"

#include <QTextDocument>
#include <QTextStream>

#include <md4qt/parser.h>

MDHighlighter::MDHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
}

QQuickTextDocument *MDHighlighter::document() const
{
    return m_quickDocument;
}

void MDHighlighter::setDocument(QQuickTextDocument *document)
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
    m_lineFormats.clear();

    if (m_active && m_quickDocument) {
        rehighlight();
    }
    Q_EMIT documentChanged();
}

bool MDHighlighter::active() const
{
    return m_active;
}

void MDHighlighter::setActive(bool active)
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

QColor MDHighlighter::textColor() const
{
    return m_theme.textColor();
}

void MDHighlighter::setTextColor(const QColor &color)
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

QColor MDHighlighter::dimmedColor() const
{
    return m_theme.dimmedColor();
}

void MDHighlighter::setDimmedColor(const QColor &color)
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

QColor MDHighlighter::codeColor() const
{
    return m_theme.codeColor();
}

void MDHighlighter::setCodeColor(const QColor &color)
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

QString MDHighlighter::fixedFontFamily() const
{
    return m_theme.fixedFontFamily();
}

void MDHighlighter::setFixedFontFamily(const QString &family)
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

qreal MDHighlighter::baseFontSize() const
{
    return m_baseFontSize;
}

void MDHighlighter::setBaseFontSize(qreal size)
{
    if (qFuzzyCompare(m_baseFontSize, size)) {
        return;
    }

    m_baseFontSize = size;
    m_dirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT baseFontSizeChanged();
}

void MDHighlighter::updateDoc()
{
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        m_lineFormats.clear();
        m_lastParsedText.clear();
        return;
    }

    const QString text = m_quickDocument->textDocument()->toPlainText();
    if (text == m_lastParsedText && !m_dirty) {
        return;
    }

    m_lineFormats.clear();
    m_lastParsedText = text;
    m_dirty = false;

    if (text.isEmpty()) {
        return;
    }

    QString textCopy = text;
    QTextStream stream(&textCopy);
    MD::Parser parser;
    auto doc = parser.parse(stream, QString(), QString());
    if (doc) {
        MDHighlighterVisitor visitor(m_theme, m_baseFontSize);
        visitor.process(doc);
        m_lineFormats = visitor.lineFormats();
    }
}

void MDHighlighter::highlightBlock(const QString &text)
{
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        return;
    }

    updateDoc();

    const int textLen = text.length();
    const int lineNum = currentBlock().blockNumber();

    const auto ranges = m_lineFormats.value(lineNum);
    for (const auto &range : ranges) {
        const int start = qMax(0, range.start);
        const int len = (range.length < 0) ? (textLen - start) : qMin(range.length, textLen - start);
        if (start < textLen && len > 0) {
            for (int i = 0; i < len; ++i) {
                QTextCharFormat current = format(start + i);
                current.merge(range.format);
                setFormat(start + i, 1, current);
            }
        }
    }
}
