// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdhighlightervisitor.h"

MDHighlighterVisitor::MDHighlighterVisitor(const MarkdownTheme &theme, qreal baseFontSize)
    : m_theme(theme)
    , m_baseFontSize(baseFontSize)
{
}

const QHash<int, QList<FormatRange>> &MDHighlighterVisitor::lineFormats() const
{
    return m_lineFormats;
}

void MDHighlighterVisitor::onAddLineEnding()
{
}

void MDHighlighterVisitor::onText(MD::Text *t)
{
    if (!t || t->startLine() < 0) {
        return;
    }

    if (t->opts() & MD::TextOption::BoldText) {
        addRange(t, m_theme.formatForInline(MDOptions::InlineStyle::Strong));
    }
    if (t->opts() & MD::TextOption::ItalicText) {
        addRange(t, m_theme.formatForInline(MDOptions::InlineStyle::Emphasis));
    }
    if (t->opts() & MD::TextOption::StrikethroughText) {
        addRange(t, m_theme.formatForInline(MDOptions::InlineStyle::Strikethrough));
    }

    addDelims(t->openStyles());
    addDelims(t->closeStyles());
}

void MDHighlighterVisitor::onMath(MD::Math *)
{
}

void MDHighlighterVisitor::onLineBreak(MD::LineBreak *)
{
}

void MDHighlighterVisitor::onHeading(MD::Heading *h)
{
    if (!h) {
        return;
    }

    addRange(h, m_theme.formatForHeading(h->level(), m_baseFontSize));
    addDelims(h->delims());

    if (h->text()) {
        onParagraph(h->text().data(), false);
    }
}

void MDHighlighterVisitor::onCode(MD::Code *c)
{
    if (!c) {
        return;
    }

    addRange(c, m_theme.formatForCodeBlock());
    addDelim(c->startDelim());
    addDelim(c->endDelim());
    addDelim(c->syntaxPos());
}

void MDHighlighterVisitor::onInlineCode(MD::Code *c)
{
    if (!c) {
        return;
    }

    addRange(c, m_theme.formatForInline(MDOptions::InlineStyle::CodeSpan));
    addDelim(c->startDelim());
    addDelim(c->endDelim());
}

void MDHighlighterVisitor::onBlockquote(MD::Blockquote *b)
{
    if (!b) {
        return;
    }

    addRange(b, m_theme.formatForBlockquote());
    addDelims(b->delims());

    MD::Visitor::onBlockquote(b);
}

void MDHighlighterVisitor::onList(MD::List *l)
{
    if (!l) {
        return;
    }

    for (const auto &item : l->items()) {
        if (item->type() == MD::ItemType::ListItem) {
            onListItem(static_cast<MD::ListItem *>(item.data()), false);
        }
    }
}

void MDHighlighterVisitor::onTable(MD::Table *)
{
}

void MDHighlighterVisitor::onAnchor(MD::Anchor *)
{
}

void MDHighlighterVisitor::onRawHtml(MD::RawHtml *)
{
}

void MDHighlighterVisitor::onHorizontalLine(MD::HorizontalLine *l)
{
    if (!l) {
        return;
    }

    addDelim(*l);
}

void MDHighlighterVisitor::onLink(MD::Link *l)
{
    if (!l) {
        return;
    }

    addRange(l->textPos(), m_theme.formatForLink());
    addDelim(l->urlPos());

    addRange(l->startLine(), l->startColumn(), l->startLine(), l->textPos().startColumn() - 1, m_theme.delimiterFormat());
    addRange(l->textPos().endLine(), l->textPos().endColumn() + 1, l->endLine(), l->endColumn(), m_theme.delimiterFormat());

    if (l->p()) {
        onParagraph(l->p().data(), false);
    }
}

void MDHighlighterVisitor::onImage(MD::Image *i)
{
    if (!i) {
        return;
    }

    addDelim(*i);
}

void MDHighlighterVisitor::onFootnoteRef(MD::FootnoteRef *)
{
}

void MDHighlighterVisitor::onListItem(MD::ListItem *i, bool first, bool skipOpeningWrap)
{
    if (!i) {
        return;
    }

    addDelim(i->delim());
    if (i->isTaskList()) {
        addDelim(i->taskDelim());
    }

    MD::Visitor::onListItem(i, first, skipOpeningWrap);
}

void MDHighlighterVisitor::addRange(const MD::WithPosition &pos, const QTextCharFormat &fmt)
{
    if (pos.startLine() >= 0) {
        addRange(pos.startLine(), pos.startColumn(), pos.endLine(), pos.endColumn(), fmt);
    }
}

void MDHighlighterVisitor::addRange(const MD::Item *item, const QTextCharFormat &fmt)
{
    if (item) {
        addRange(*item, fmt);
    }
}

void MDHighlighterVisitor::addDelim(const MD::WithPosition &pos)
{
    addRange(pos, m_theme.delimiterFormat());
}

void MDHighlighterVisitor::addRange(int startLine, int startCol, int endLine, int endCol, const QTextCharFormat &fmt)
{
    if (startLine < 0 || endLine < 0 || startLine > endLine) {
        return;
    }

    for (int line = startLine; line <= endLine; ++line) {
        const int colStart = (line == startLine) ? qMax(0, startCol) : 0;
        const int length = (line == endLine) ? (endCol - colStart + 1) : -1;
        if (length != 0) {
            m_lineFormats[line].append({colStart, length, fmt});
        }
    }
}
