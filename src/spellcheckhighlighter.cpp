// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "spellcheckhighlighter.h"
#include "qregularexpression.h"

#include <QQmlFile>
#include <QQmlFileSelector>
#include <QStringBuilder>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>

#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>

// #include "documenthandler.h"

// #include "chatdocumenthandler_logging.h"

SyntaxHighlighter::SyntaxHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    mentionFormat.setFontWeight(QFont::Bold);
    mentionFormat.setForeground(Qt::blue);

    errorFormat.setForeground(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    connect(checker, &Sonnet::BackgroundChecker::misspelling, this, [this](const QString &word, int start) {
        errors += {start, word};
        checker->continueChecking();
    });
    connect(checker, &Sonnet::BackgroundChecker::done, this, [this]() {
        rehighlightTimer.start();
    });
    rehighlightTimer.setInterval(100);
    rehighlightTimer.setSingleShot(true);
    rehighlightTimer.callOnTimeout(this, &QSyntaxHighlighter::rehighlight);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    //    if (text != previousText) {
    //        previousText = text;
    //        checker->stop();
    //        errors.clear();
    //        checker->setText(text);
    //    }
    //    for (const auto &error : errors) {
    //        setFormat(error.first, error.second.size(), errorFormat);
    //    }
    QTextCharFormat latexFormat;
    latexFormat.setFontWeight(QFont::Bold);
    latexFormat.setForeground(Qt::darkMagenta);

    QRegularExpression expression(QString::fromUtf8("\$.+?\$"));
    QRegularExpressionMatchIterator i = expression.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        setFormat(match.capturedStart(), match.capturedLength(), latexFormat);
    }

    //        auto handler = dynamic_cast<DocumentHandler *>(parent());
    //        auto room = handler->room();
    //        if (!room) {
    //            return;
    //        }
    //        auto mentions = handler->chatBarCache()->mentions();
    //        mentions->erase(std::remove_if(mentions->begin(),
    //                                       mentions->end(),
    //                                       [this](auto &mention) {
    //                                           if (document()->toPlainText().isEmpty()) {
    //                                               return false;
    //                                           }

    //                                           if (mention.cursor.position() == 0 && mention.cursor.anchor() == 0) {
    //                                               return true;
    //                                           }

    //                                           if (mention.cursor.position() - mention.cursor.anchor() != mention.text.size()) {
    //                                               mention.cursor.setPosition(mention.start);
    //                                               mention.cursor.setPosition(mention.cursor.anchor() + mention.text.size(), QTextCursor::KeepAnchor);
    //                                           }

    //                                           if (mention.cursor.selectedText() != mention.text) {
    //                                               return true;
    //                                           }
    //                                           if (currentBlock() == mention.cursor.block()) {
    //                                               mention.start = mention.cursor.anchor();
    //                                               mention.position = mention.cursor.position();
    //                                               setFormat(mention.cursor.selectionStart(), mention.cursor.selectedText().size(), mentionFormat);
    //                                           }
    //                                           return false;
    //                                       }),
    //                        mentions->end());
}
