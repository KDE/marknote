// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MDSERIALIZER_H
#define MDSERIALIZER_H

#include <QHash>
#include <md4qt/doc.h>
#include <md4qt/visitor.h>
#include <memory>

class MdVisitor : public MD::Visitor
{
public:
    MdVisitor();
    ~MdVisitor() override;

    virtual QString toMd(QSharedPointer<MD::Document> doc);

protected:
    virtual void openStyle(const typename MD::ItemWithOpts::Styles &styles);

    virtual void closeStyle(const typename MD::ItemWithOpts::Styles &styles);

    void onAddLineEnding() override;

    void onText(MD::Text *t) override;

    void onMath(MD::Math *m) override;

    void onLineBreak(MD::LineBreak *) override;

    void onParagraph(MD::Paragraph *p, bool wrap, bool skipOpeningWrap = false) override;

    void onHeading(MD::Heading *h) override;

    void onCode(MD::Code *c) override;

    void onInlineCode(MD::Code *c) override;

    void onBlockquote(MD::Blockquote *b) override;

    void onList(MD::List *l) override;

    void onTable(MD::Table *t) override;

    void onAnchor(MD::Anchor *a) override;

    void onRawHtml(MD::RawHtml *h) override;

    void onHorizontalLine(MD::HorizontalLine *) override;

    void onLink(MD::Link *l) override;

    void onImage(MD::Image *i) override;

    void onFootnoteRef(MD::FootnoteRef *ref) override;

    void onListItem(MD::ListItem *i, bool first, bool skipOpeningWrap = false) override;

    virtual void onHeading(MD::Heading *h, const QString &ht);

    virtual void onFootnotes(const QString &footnoteBackLinkContent);

    void addNewBlock();
    void trimEnd();

protected:
    std::unique_ptr<QString> m_Md;

    bool m_dontIncrementFootnoteCount = false;
    int m_listIndentLevel = 0;

    struct FootnoteRefStuff {
        QString m_id;
        qsizetype m_count = 0;
        qsizetype m_current = 0;
    };

    QVector<FootnoteRefStuff> m_fns;
};

#endif // MDSERIALIZER_H
