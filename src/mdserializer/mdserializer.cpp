// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdserializer.h"
#include <QRegularExpression>
using namespace Qt::StringLiterals;

MDSerializer::MDSerializer() = default;

MDSerializer::~MDSerializer() = default;

QString MDSerializer::processDoc(QSharedPointer<MD::Document> doc)
{
    m_Md = std::make_unique<QString>();

    this->process(doc);

    return *m_Md;
}

QString MDSerializer::toMd(QSharedPointer<MD::Item> item)
{
    MDSerializer visitor;

    auto doc = QSharedPointer<MD::Document>::create();
    doc->appendItem(item);

    auto result = visitor.processDoc(doc);

    while (result.endsWith(u'\n')) {
        result.chop(1);
    }

    return result;
}

void MDSerializer::openStyle(const typename MD::ItemWithOpts::Styles &styles)
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

void MDSerializer::closeStyle(const typename MD::ItemWithOpts::Styles &styles)
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

void MDSerializer::onAddLineEnding()
{
    m_Md->push_back(u"\n"_s);
}

void MDSerializer::onText(MD::Text *t)
{
    openStyle(t->openStyles());

    m_Md->push_back(t->text());

    closeStyle(t->closeStyles());
}

void MDSerializer::onMath(MD::Math *m)
{
    openStyle(m->openStyles());

    m_Md->push_back(m->isInline() ? u"$"_s : u"$$"_s);
    m_Md->push_back(m->expr());
    m_Md->push_back(m->isInline() ? u"$"_s : u"$$"_s);

    closeStyle(m->closeStyles());
}

void MDSerializer::onLineBreak(MD::LineBreak *)
{
    m_Md->push_back(u"<br>"_s);
}

void MDSerializer::onParagraph(MD::Paragraph *p, bool wrap, bool skipOpeningWrap)
{
    Visitor::onParagraph(p, wrap);

    if (wrap) {
        addNewBlock();
    }
}

void MDSerializer::onHeading(MD::Heading *h)
{
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
}

void MDSerializer::onCode(MD::Code *c)
{
    m_Md->push_back(u"```"_s);

    if (!c->syntax().isEmpty()) {
        m_Md->push_back(c->syntax());
    }

    m_Md->push_back(u"\n"_s);
    m_Md->push_back(c->text());
    m_Md->push_back(u"\n```"_s);
    addNewBlock();
}

void MDSerializer::onInlineCode(MD::Code *c)
{
    openStyle(c->openStyles());

    m_Md->push_back(u"`"_s);

    m_Md->push_back(c->text());

    m_Md->push_back(u"`"_s);

    closeStyle(c->closeStyles());
}

void MDSerializer::onBlockquote(MD::Blockquote *b)
{
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
}

void MDSerializer::onListItem(MD::ListItem *i, bool first, bool skipOpeningWrap)
{
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
}

void MDSerializer::onList(MD::List *l)
{
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
}

void MDSerializer::onTable(MD::Table *t)
{
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
}

void MDSerializer::onAnchor(MD::Anchor *a)
{
}

void MDSerializer::onRawHtml(MD::RawHtml *h)
{
    openStyle(h->openStyles());

    m_Md->push_back(h->text());

    closeStyle(h->closeStyles());

    addNewBlock();
}

void MDSerializer::onHorizontalLine(MD::HorizontalLine *)
{
    m_Md->push_back(u"---"_s);
    addNewBlock();
}

void MDSerializer::onLink(MD::Link *l)
{
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
}

void MDSerializer::onImage(MD::Image *i)
{
    openStyle(i->openStyles());

    m_Md->push_back(u"!["_s);
    m_Md->push_back(i->text());
    m_Md->push_back(u"]("_s);
    m_Md->push_back(i->url());
    m_Md->push_back(u")"_s);

    closeStyle(i->closeStyles());
}

void MDSerializer::onFootnoteRef(MD::FootnoteRef *ref)
{
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
}

void MDSerializer::onHeading(MD::Heading *h, const QString &ht)
{
    QString hashes(h->level(), u'#');
    *m_Md += hashes + u" "_s;

    if (h->text().get()) {
        onParagraph(h->text().get(), false);
    }

    addNewBlock();
}

void MDSerializer::onFootnotes(const QString &footnoteBackLinkContent)
{
}

void MDSerializer::addNewBlock()
{
    m_Md->push_back(u"\n\n"_s);
}

void MDSerializer::trimEnd()
{
    while (m_Md->endsWith(u'\n')) {
        m_Md->chop(1);
    }
}
