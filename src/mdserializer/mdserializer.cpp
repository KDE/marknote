// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdserializer.h"
#include <QDebug>
#include <QRegularExpression>
using namespace Qt::StringLiterals;

MdVisitor::MdVisitor() = default;

MdVisitor::~MdVisitor() = default;

QString MdVisitor::toMd(QSharedPointer<MD::Document> doc)
{
    m_Md = std::make_unique<QString>();

    this->process(doc);

    return *m_Md;
}

void MdVisitor::openStyle(const typename MD::ItemWithOpts::Styles &styles)
{
    for (const auto &s : styles) {
        switch (s.style()) {
        case MD::TextOption::BoldText:
            m_Md->push_back(u"**"_s);
            break;

        case MD::TextOption::ItalicText:
            m_Md->push_back(u"*"_s);
            break;

        case MD::TextOption::StrikethroughText:
            m_Md->push_back(u"~"_s);
            break;

        default:
            break;
        }
    }
}

void MdVisitor::closeStyle(const typename MD::ItemWithOpts::Styles &styles)
{
    for (const auto &s : styles) {
        switch (s.style()) {
        case MD::TextOption::BoldText:
            m_Md->push_back(u"**"_s);
            break;

        case MD::TextOption::ItalicText:
            m_Md->push_back(u"*"_s);
            break;

        case MD::TextOption::StrikethroughText:
            m_Md->push_back(u"~"_s);
            break;

        default:
            break;
        }
    }
}

void MdVisitor::onAddLineEnding()
{
    qDebug() << "AddLineEnding Enter";
    m_Md->push_back(u"\n"_s);
    qDebug() << "AddLineEnding Exit";
}

void MdVisitor::onText(MD::Text *t)
{
    qDebug() << "Text Enter";
    openStyle(t->openStyles());

    m_Md->push_back(t->text());

    closeStyle(t->closeStyles());
    qDebug() << "Text Exit";
}

void MdVisitor::onMath(MD::Math *m)
{
    qDebug() << "Math Enter";
    openStyle(m->openStyles());

    m_Md->push_back(m->isInline() ? u"$"_s : u"$$"_s);
    m_Md->push_back(m->expr());
    m_Md->push_back(m->isInline() ? u"$"_s : u"$$"_s);

    closeStyle(m->closeStyles());
    qDebug() << "Math Exit";
}

void MdVisitor::onLineBreak(MD::LineBreak *)
{
    qDebug() << "LineBreak Enter";
    m_Md->push_back(u"<br>"_s);
    qDebug() << "LineBreak Exit";
}

void MdVisitor::onParagraph(MD::Paragraph *p, bool wrap, bool skipOpeningWrap)
{
    qDebug() << "Paragraph Enter";
    Visitor::onParagraph(p, wrap);

    if (wrap) {
        addNewBlock();
    }

    qDebug() << "Paragraph Exit";
}

void MdVisitor::onHeading(MD::Heading *h)
{
    qDebug() << "Heading Enter";
    switch (h->level()) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        onHeading(h, u"h"_s + QString::number(h->level()));
        break;
    default:
        break;
    }
    qDebug() << "Heading Exit";
}

void MdVisitor::onCode(MD::Code *c)
{
    qDebug() << "Code Enter";
    m_Md->push_back(u"```"_s);

    if (!c->syntax().isEmpty()) {
        m_Md->push_back(c->syntax());
    }

    m_Md->push_back(u"\n"_s);
    m_Md->push_back(c->text());
    m_Md->push_back(u"\n```"_s);
    addNewBlock();
    qDebug() << "Code Exit";
}

void MdVisitor::onInlineCode(MD::Code *c)
{
    qDebug() << "InlineCode Enter";
    openStyle(c->openStyles());

    m_Md->push_back(u"`"_s);

    m_Md->push_back(c->text());

    m_Md->push_back(u"`"_s);

    closeStyle(c->closeStyles());
    qDebug() << "InlineCode Exit";
}

void MdVisitor::onBlockquote(MD::Blockquote *b)
{
    qDebug() << "Blockquote Enter";
    std::unique_ptr<QString> curStr = std::move(m_Md);

    m_Md = std::make_unique<QString>();

    Visitor::onBlockquote(b);

    trimEnd();

    QStringList lines = m_Md->split(u'\n');
    for (auto &line : lines) {
        line.prepend(u"> "_s);
    }

    *curStr += lines.join(u'\n');

    m_Md = std::move(curStr);
    addNewBlock();
    qDebug() << "Blockquote Exit";
}

void MdVisitor::onListItem(MD::ListItem *i, bool first, bool skipOpeningWrap)
{
    qDebug() << "ListItem Enter";
    std::unique_ptr<QString> curStr = std::move(m_Md);
    m_Md = std::make_unique<QString>();

    if (i->listType() == MD::ListItem::Ordered) {
        m_Md->push_back(u"1. "_s);
    } else {
        m_Md->push_back(u"- "_s);
    }

    if (i->isTaskList()) {
        skipOpeningWrap = Visitor::wrapFirstParagraphInListItem(i);

        m_Md->push_back(u'[');

        if (i->isChecked()) {
            m_Md->push_back(u'x');
        } else {
            m_Md->push_back(u' ');
        }

        m_Md->push_back(u"] "_s);
    }

    Visitor::onListItem(i, first, skipOpeningWrap);

    trimEnd();

    QStringList lines = m_Md->split(u'\n');

    bool firstLine = true;
    for (auto &line : lines) {
        if (!firstLine) {
            QRegularExpression listItemRegex(u"^\\s*([-*+] |\\d+\\. )"_s);

            if (!listItemRegex.match(line).hasMatch()) {
                line.prepend(QString(2, u' '));
            }
        } else if (!wrapFirstParagraphInListItem(i)) {
            // we need to insert a line break before the first line if it is not wrapped in a paragraph
            int index = line.indexOf(u"    "_s);
            if (index != -1) {
                line.insert(index, u'\n');
            }
        }

        firstLine = false;
    }

    *curStr += lines.join(u'\n') + u"\n"_s;

    m_Md = std::move(curStr);
    qDebug() << "ListItem Exit";
}

void MdVisitor::onList(MD::List *l)
{
    qDebug() << "List Enter";
    bool first = true;

    std::unique_ptr<QString> curStr = std::move(m_Md);
    m_Md = std::make_unique<QString>();

    m_listIndentLevel++;

    for (auto it = l->items().cbegin(), last = l->items().cend(); it != last; ++it) {
        if ((*it)->type() == MD::ItemType::ListItem) {
            auto *item = static_cast<MD::ListItem *>(it->get());

            onListItem(item, first);

            first = false;
        }
    }

    if (m_listIndentLevel > 1) {
        m_listIndentLevel--;

        trimEnd();

        QStringList lines = m_Md->split(u'\n');
        for (auto &line : lines) {
            line.prepend(QString(4, u' '));
        }

        *curStr += lines.join(u'\n') + u"\n"_s;
    } else {
        if (m_listIndentLevel > 0) {
            m_listIndentLevel--;
        }

        *curStr += *m_Md + u"\n"_s;
    }

    m_Md = std::move(curStr);
    qDebug() << "List Exit";
}

void MdVisitor::onTable(MD::Table *t)
{
    qDebug() << "Table Enter";

    if (!t->isEmpty() && !t->rows().empty()) {
        int columns = 0;

        m_Md->push_back(u"|"_s);
        for (auto th = (*t->rows().cbegin())->cells().cbegin(), last = (*t->rows().cbegin())->cells().cend(); th != last; ++th) {
            m_Md->push_back(u" "_s);

            std::unique_ptr<QString> curStr = std::move(m_Md);
            m_Md = std::make_unique<QString>();

            this->onTableCell(th->get());
            trimEnd();

            QString cellContent = *m_Md;
            cellContent.replace(u"\n\n"_s, u"<br>"_s);
            cellContent.replace(u'\n', u' ');

            *curStr += cellContent;
            m_Md = std::move(curStr);

            m_Md->push_back(u" |"_s);
            ++columns;
        }
        m_Md->push_back(u"\n"_s);

        m_Md->push_back(u"|"_s);
        for (int i = 0; i < columns; ++i) {
            m_Md->push_back(u" --- |"_s);
        }
        m_Md->push_back(u"\n"_s);

        for (auto r = std::next(t->rows().cbegin()), rlast = t->rows().cend(); r != rlast; ++r) {
            m_Md->push_back(u"|"_s);

            int i = 0;

            for (auto c = (*r)->cells().cbegin(), clast = (*r)->cells().cend(); c != clast; ++c) {
                m_Md->push_back(u" "_s);

                std::unique_ptr<QString> curStr = std::move(m_Md);
                m_Md = std::make_unique<QString>();

                this->onTableCell(c->get());
                trimEnd();

                QString cellContent = *m_Md;
                cellContent.replace(u"\n\n"_s, u"<br>"_s);
                cellContent.replace(u'\n', u' ');

                *curStr += cellContent;
                m_Md = std::move(curStr);

                m_Md->push_back(u" |"_s);

                ++i;

                if (i == columns) {
                    break;
                }
            }

            for (; i < columns; ++i) {
                m_Md->push_back(u"   |"_s);
            }

            m_Md->push_back(u"\n"_s);
        }
    }

    addNewBlock();
    qDebug() << "Table Exit";
}

void MdVisitor::onAnchor(MD::Anchor *a)
{
    qDebug() << "Anchor Enter";
    qDebug() << "Anchor Exit";
}

void MdVisitor::onRawHtml(MD::RawHtml *h)
{
    qDebug() << "RawHtml Enter";
    openStyle(h->openStyles());

    m_Md->push_back(h->text());

    closeStyle(h->closeStyles());

    addNewBlock();
    qDebug() << "RawHtml Exit";
}

void MdVisitor::onHorizontalLine(MD::HorizontalLine *)
{
    qDebug() << "HorizontalLine Enter";
    m_Md->push_back(u"---"_s);
    addNewBlock();
    qDebug() << "HorizontalLine Exit";
}

void MdVisitor::onLink(MD::Link *l)
{
    qDebug() << "Link Enter";
    QString url = l->url();

    const auto lit = this->m_doc->labeledLinks().find(url);

    if (lit != this->m_doc->labeledLinks().cend()) {
        url = (*lit)->url();
    }

    if (std::find(this->m_anchors.cbegin(), this->m_anchors.cend(), url) != this->m_anchors.cend()) {
        url = u"#"_s + url;
    } else if (url.startsWith(u"#"_s)) {
        const auto it = this->m_doc->labeledHeadings().find(url);

        if (it == this->m_doc->labeledHeadings().cend()) {
            auto path = static_cast<MD::Anchor *>(this->m_doc->items().at(0).get())->label();
            const auto sp = path.lastIndexOf(u"/"_s);
            path.remove(sp, path.length() - sp);
            const auto p = url.indexOf(path) - 1;
            url.remove(p, url.length() - p);
        } else {
            url = (*it)->label();
        }
    }

    openStyle(l->openStyles());

    m_Md->push_back(u'[');

    if (l->p() && !l->p()->isEmpty()) {
        onParagraph(l->p().get(), false);
    } else if (!l->img()->isEmpty()) {
        onImage(l->img().get());
    } else if (!l->text().isEmpty()) {
        m_Md->push_back(l->text());
    } else {
        m_Md->push_back(l->url());
    }

    m_Md->push_back(u"]("_s);
    m_Md->push_back(url);
    m_Md->push_back(u")"_s);

    closeStyle(l->closeStyles());
    qDebug() << "Link Exit";
}

void MdVisitor::onImage(MD::Image *i)
{
    qDebug() << "Image Enter";
    openStyle(i->openStyles());

    m_Md->push_back(u"!["_s);
    m_Md->push_back(i->text());
    m_Md->push_back(u"]("_s);
    m_Md->push_back(i->url());
    m_Md->push_back(u")"_s);

    closeStyle(i->closeStyles());
    qDebug() << "Image Exit";
}

void MdVisitor::onFootnoteRef(MD::FootnoteRef *ref)
{
    qDebug() << "FootnoteRef Enter";
    const auto fit = this->m_doc->footnotesMap().find(ref->id());

    if (fit != this->m_doc->footnotesMap().cend()) {
        const auto r = std::find_if(m_fns.begin(), m_fns.end(), [&ref](const auto &stuff) {
            return ref->id() == stuff.m_id;
        });

        openStyle(ref->openStyles());

        m_Md->push_back(u"<sup>"_s);
        m_Md->push_back(u"<a href=\"#"_s);
        m_Md->push_back(ref->id());
        m_Md->push_back(u"\" id=\"ref-"_s);
        m_Md->push_back(ref->id());
        m_Md->push_back(u"-"_s);

        if (r == m_fns.end()) {
            m_Md->push_back(u"1"_s);
        } else {
            m_Md->push_back(QString::number(++(r->m_current)));

            if (!m_dontIncrementFootnoteCount) {
                ++(r->m_count);
            }
        }

        m_Md->push_back(u"\">"_s);

        if (r == m_fns.end()) {
            m_Md->push_back(QString::number(m_fns.size() + 1));

            m_fns.push_back({ref->id(), 1, 1});
        } else {
            m_Md->push_back(QString::number(std::distance(m_fns.begin(), r) + 1));
        }

        m_Md->push_back(u"</a></sup>"_s);

        closeStyle(ref->closeStyles());
    } else {
        onText(static_cast<MD::Text *>(ref));
    }
    qDebug() << "FootnoteRef Exit";
}

void MdVisitor::onHeading(MD::Heading *h, const QString &ht)
{
    QString hashes(h->level(), u'#');
    *m_Md += hashes + u" "_s;

    if (h->text().get()) {
        onParagraph(h->text().get(), false);
    }

    addNewBlock();
}

void MdVisitor::onFootnotes(const QString &footnoteBackLinkContent)
{
    qDebug() << "Footnotes Enter";
    qDebug() << "Footnotes Exit";
}

void MdVisitor::addNewBlock()
{
    m_Md->push_back(u"\n\n"_s);
}

void MdVisitor::trimEnd()
{
    while (m_Md->endsWith(u'\n')) {
        m_Md->chop(1);
    }
}
