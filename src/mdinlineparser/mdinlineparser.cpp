// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mdinlineparser.h"

#include <md4qt/asterisk_emphasis_parser.h>
#include <md4qt/doc.h>
#include <md4qt/inline_code_parser.h>
#include <md4qt/paragraph_parser.h>
#include <md4qt/parser.h>
#include <md4qt/strikethrough_emphasis_parser.h>
#include <md4qt/underline_emphasis_parser.h>

#include <QTextStream>

MDInlineStyleParser::MDInlineStyleParser(QObject *parent)
    : QObject(parent)
{
}

QList<MDInlineSpan> MDInlineStyleParser::parse(const QString &text)
{
    QList<MDInlineSpan> spans;

    QList<int> lineOffsets;
    lineOffsets.append(0);
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == u'\n') {
            lineOffsets.append(i + 1);
        }
    }

    MD::Parser parser;
    MD::Parser::BlockParsers blockParsers;
    MD::Parser::appendBlockParser<MD::ParagraphParser>(blockParsers, &parser);
    parser.setBlockParsers(blockParsers);

    MD::Parser::InlineParsers inlineParsers;
    MD::Parser::appendInlineParser<MD::InlineCodeParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::AsteriskEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::UnderlineEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::StrikethroughEmphasisParser>(inlineParsers);
    parser.setInlineParsers(inlineParsers);

    QString textCopy = text;
    QTextStream stream(&textCopy);
    QSharedPointer<MD::Document> doc = parser.parse(stream, QString(), QString());

    struct ActiveStyle {
        int start;
        MDOptions::InlineStyle type;
        int origStyle;
    };
    QList<ActiveStyle> activeStyles;

    for (const auto &blockItem : doc->items()) {
        if (blockItem->type() == MD::ItemType::Paragraph) {
            auto *paragraph = static_cast<MD::Paragraph *>(blockItem.get());

            for (const auto &inlineItem : paragraph->items()) {
                if (inlineItem->type() == MD::ItemType::Text) {
                    auto *textItem = static_cast<MD::Text *>(inlineItem.get());

                    for (const auto &openStyle : textItem->openStyles()) {
                        ActiveStyle as;
                        as.start = lineOffsets.value(openStyle.startLine(), 0) + openStyle.startColumn();
                        as.origStyle = openStyle.style();
                        switch (openStyle.style()) {
                        case MD::TextOption::ItalicText:
                            as.type = MDOptions::InlineStyle::Emphasis;
                            break;
                        case MD::TextOption::BoldText:
                            as.type = MDOptions::InlineStyle::Strong;
                            break;
                        case MD::TextOption::StrikethroughText:
                            as.type = MDOptions::InlineStyle::Strikethrough;
                            break;
                        default:
                            continue;
                        }
                        activeStyles.append(as);
                    }

                    for (const auto &closeStyle : textItem->closeStyles()) {
                        for (int i = activeStyles.size() - 1; i >= 0; --i) {
                            if (activeStyles[i].origStyle == closeStyle.style()) {
                                MDInlineSpan span;
                                span.start = activeStyles[i].start;
                                span.end = lineOffsets.value(closeStyle.endLine(), 0) + closeStyle.endColumn();
                                span.type = activeStyles[i].type;
                                spans.append(span);
                                activeStyles.removeAt(i);
                                break;
                            }
                        }
                    }
                } else if (inlineItem->type() == MD::ItemType::Code) {
                    auto *codeItem = static_cast<MD::Code *>(inlineItem.get());
                    if (codeItem->isInline()) {
                        MDInlineSpan span;
                        span.start = lineOffsets.value(codeItem->startLine(), 0) + codeItem->startColumn();
                        span.end = lineOffsets.value(codeItem->endLine(), 0) + codeItem->endColumn();
                        span.type = MDOptions::InlineStyle::CodeSpan;
                        spans.append(span);
                    }
                }
            }
        }
    }

    return spans;
}
