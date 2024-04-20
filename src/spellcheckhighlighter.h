// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
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

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT
public:
    QTextCharFormat mentionFormat;
    QTextCharFormat errorFormat;
    Sonnet::BackgroundChecker *checker = new Sonnet::BackgroundChecker;
    Sonnet::Settings settings;
    QList<QPair<int, QString>> errors;
    QString previousText;
    QTimer rehighlightTimer;
    SyntaxHighlighter(QObject *parent = nullptr);

    void highlightBlock(const QString &text) override;
};
