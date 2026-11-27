// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdhighlighter.h"
#include "documenthandler.h"
#include "mdhighlightervisitor.h"

#include <KColorScheme>
#include <KColorUtils>

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTextStream>

#include <md4qt/parser.h>

MDHighlighter::MDHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    initTheme();

    qGuiApp->installEventFilter(this);
}

QQuickTextDocument *MDHighlighter::document() const
{
    return m_quickDocument;
}

void MDHighlighter::initTheme()
{
    KColorScheme scheme(QPalette::Active, KColorScheme::View);

    m_theme.setTextColor(scheme.foreground(KColorScheme::NormalText).color());
    m_theme.setDimmedColor(scheme.foreground(KColorScheme::InactiveText).color());

    QColor tint = scheme.decoration(KColorScheme::FocusColor).color();
    tint.setAlphaF(tint.alphaF() * 0.7);
    m_theme.setCodeColor(KColorUtils::overlayColors(m_theme.textColor(), tint));

    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection);
    m_theme.setAccentColor(selectionScheme.background(KColorScheme::NormalBackground).color());
}

int MDHighlighter::firstVisibleBlockNumber() const
{
    if (!m_quickDocument) {
        return -1;
    }

    QTextDocument *doc = m_quickDocument->textDocument();

    if (!doc) {
        return -1;
    }

    QAbstractTextDocumentLayout *layout = doc->documentLayout();

    if (!layout) {
        return -1;
    }

    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        QRectF rect = layout->blockBoundingRect(block);

        if (rect.bottom() > m_contentY) {
            return block.blockNumber();
        }
    }

    return -1;
}

bool MDHighlighter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qGuiApp && event->type() == QEvent::ApplicationPaletteChange) {
        initTheme();

        updateBlockFormats();
    }

    return QSyntaxHighlighter::eventFilter(watched, event);
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
        parseDoc();
        updateBlockFormats();

        connect(m_quickDocument->textDocument(), &QTextDocument::contentsChange, this, [this](int, int charsRemoved, int charsAdded) {
            if (charsRemoved > 0 || charsAdded > 0) {
                parseDoc();
                updateBlockFormats();
            }
        });
    } else {
        QSyntaxHighlighter::setDocument(nullptr);
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
        doRehighlight();
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
    if (m_active) {
        doRehighlight();
    }
    Q_EMIT modeChanged();
}

MDHighlighter::RehighlightMode MDHighlighter::rehighlightMode() const
{
    return m_rehighlightMode;
}

void MDHighlighter::setRehighlightMode(RehighlightMode mode)
{
    if (m_rehighlightMode == mode) {
        return;
    }

    m_rehighlightMode = mode;

    Q_EMIT rehighlightModeChanged();
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
    updateBlockFormats();

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

    updateBlockFormats();

    Q_EMIT baseFontSizeChanged();
}

void MDHighlighter::doRehighlight()
{
    if (m_rehighlightMode == FullRehighlight) {
        rehighlight();
    } else {
        rehighlightVisible();
    }
}

void MDHighlighter::rehighlightVisible()
{
    const auto i = firstVisibleBlockNumber();

    if (i >= 0) {
        QTextDocument *doc = m_quickDocument->textDocument();

        if (!doc) {
            return;
        }

        QAbstractTextDocumentLayout *layout = doc->documentLayout();

        if (!layout) {
            return;
        }

        QTextBlock b = doc->findBlockByNumber(i);

        while (b.isValid()) {
            QRectF rect = layout->blockBoundingRect(b);

            rehighlightBlock(b);

            if (rect.bottom() > m_contentY + m_contentHeight) {
                break;
            }

            b = b.next();
        }
    }
}

void MDHighlighter::updateViewportInfo(int contentY, int height)
{
    m_contentY = contentY;
    m_contentHeight = height;
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

    MDHighlighterVisitor visitor(m_quickDocument->textDocument(), m_theme, m_baseFontSize, m_lineOffsets, m_mode);
    visitor.process(m_doc);
    m_blockFormats = visitor.takeBlockFormats();

    if (m_active) {
        doRehighlight();
    }
}

void MDHighlighter::highlightBlock(const QString &text)
{
    Q_UNUSED(text)
    if (!m_quickDocument || !m_quickDocument->textDocument()) {
        return;
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
