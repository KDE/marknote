// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdhighlighter.h"
#include "documenthandler.h"

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
            m_textDirty = true;
            m_formatsDirty = true;
        });
    } else {
        QSyntaxHighlighter::setDocument(nullptr);
    }

    m_textDirty = true;
    m_formatsDirty = true;

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
        m_formatsDirty = true;
        rehighlight();
    }
    Q_EMIT activeChanged();
}

MDHighlighter::Mode MDHighlighter::mode() const
{
    return m_mode;
}

void MDHighlighter::setMode(Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    m_formatsDirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT modeChanged();
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
    m_formatsDirty = true;
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
    m_formatsDirty = true;
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
    m_formatsDirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT codeColorChanged();
}

QColor MDHighlighter::accentColor() const
{
    return m_theme.accentColor();
}

void MDHighlighter::setAccentColor(const QColor &color)
{
    if (m_theme.accentColor() == color) {
        return;
    }

    m_theme.setAccentColor(color);
    m_formatsDirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT accentColorChanged();
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
    m_formatsDirty = true;
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
    m_formatsDirty = true;
    if (m_active) {
        rehighlight();
    }
    Q_EMIT baseFontSizeChanged();
}

void MDHighlighter::parseDoc()
{
    m_doc.reset();
    m_lineOffsets.clear();

    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        return;
    }

    const QString text = replaceLineEndings(m_quickDocument->textDocument()->toRawText());
    if (text.isEmpty()) {
        return;
    }

    m_lineOffsets.append(0);
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == u'\n' || text[i] == QChar::LineSeparator) {
            m_lineOffsets.append(i + 1);
        }
    }

    QString textCopy = text;
    textCopy.replace(QChar::LineSeparator, u'\n');
    QTextStream stream(&textCopy);
    MD::Parser parser;
    m_doc = parser.parse(stream, QString(), QString());
}

void MDHighlighter::updateBlockFormats()
{
    m_blockFormats.clear();

    if (!m_quickDocument || !m_quickDocument->textDocument() || !m_doc) {
        return;
    }

    MDHighlighterVisitor visitor(m_quickDocument->textDocument(), m_theme, m_baseFontSize, m_lineOffsets, static_cast<HighlighterMode>(m_mode));
    visitor.process(m_doc);
    m_blockFormats = visitor.takeBlockFormats();
}

void MDHighlighter::highlightBlock(const QString &text)
{
    Q_UNUSED(text)
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        return;
    }

    if (m_textDirty) {
        parseDoc();
        m_textDirty = false;
        m_formatsDirty = true;
    }

    if (m_formatsDirty) {
        updateBlockFormats();
        m_formatsDirty = false;
    }

    const int blockNum = currentBlock().blockNumber();
    if (blockNum >= 0 && blockNum < m_blockFormats.size()) {
        for (const auto &range : m_blockFormats.at(blockNum)) {
            applyFormat(range.start, range.length, range.format);
        }
    }
}

void MDHighlighter::applyFormat(int start, int length, const QTextCharFormat &fmt)
{
    for (int i = 0; i < length; ++i) {
        QTextCharFormat current = format(start + i);
        current.merge(fmt);
        setFormat(start + i, 1, current);
    }
}
