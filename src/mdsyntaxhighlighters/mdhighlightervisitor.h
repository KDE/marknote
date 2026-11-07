// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDHIGHLIGHTERVISITOR_H
#define MDHIGHLIGHTERVISITOR_H

#include "markdowntheme.h"

#include <QHash>
#include <QList>
#include <QTextCharFormat>

#include <md4qt/doc.h>
#include <md4qt/visitor.h>

struct FormatRange {
    int start = 0;
    int end = 0;
    QTextCharFormat format;
};

class MDHighlighterVisitor : public MD::Visitor
{
public:
    MDHighlighterVisitor(const MarkdownTheme &theme, qreal baseFontSize, const QList<int> &lineOffsets);
    ~MDHighlighterVisitor() override = default;

    const QList<FormatRange> &ranges() const;

protected:
    void onAddLineEnding() override;
    void onText(MD::Text *t) override;
    void onMath(MD::Math *) override;
    void onLineBreak(MD::LineBreak *b) override;
    void onHeading(MD::Heading *h) override;
    void onCode(MD::Code *c) override;
    void onInlineCode(MD::Code *c) override;
    void onBlockquote(MD::Blockquote *b) override;
    void onList(MD::List *l) override;
    void onTable(MD::Table *t) override;
    void onAnchor(MD::Anchor *a) override;
    void onRawHtml(MD::RawHtml *h) override;
    void onHorizontalLine(MD::HorizontalLine *l) override;
    void onLink(MD::Link *l) override;
    void onImage(MD::Image *i) override;
    void onFootnoteRef(MD::FootnoteRef *ref) override;
    void onFootnote(MD::Footnote *f) override;
    void onListItem(MD::ListItem *i, bool first, bool skipOpeningWrap = false, MD::ListLooseness looseness = MD::ListLooseness::Unknown) override;

private:
    void addRange(const MD::WithPosition &pos, const QTextCharFormat &fmt);
    void addRange(const MD::Item *item, const QTextCharFormat &fmt);
    void addDelim(const MD::WithPosition &pos);

    template<typename Container>
    void addDelims(const Container &delims)
    {
        for (const auto &d : delims) {
            addDelim(d);
        }
    }

    void addRange(int startLine, int startCol, int endLine, int endCol, const QTextCharFormat &fmt);

    const MarkdownTheme &m_theme;
    qreal m_baseFontSize;
    const QList<int> &m_lineOffsets;
    QList<FormatRange> m_ranges;
};

#endif // MDHIGHLIGHTERVISITOR_H
