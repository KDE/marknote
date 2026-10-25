// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MARKDOWNTHEME_H
#define MARKDOWNTHEME_H

#include "mdoptions/mdoptions.h"

#include <QColor>
#include <QFont>
#include <QString>
#include <QTextCharFormat>

class MarkdownTheme
{
public:
    MarkdownTheme();

    QColor textColor() const;
    void setTextColor(const QColor &color);

    QColor dimmedColor() const;
    void setDimmedColor(const QColor &color);

    QColor codeColor() const;
    void setCodeColor(const QColor &color);

    QString fixedFontFamily() const;
    void setFixedFontFamily(const QString &family);

    QTextCharFormat formatForInline(MDOptions::InlineStyle type) const;
    QTextCharFormat delimiterFormat() const;

private:
    QColor m_textColor;
    QColor m_dimmedColor;
    QColor m_codeColor;
    QString m_fixedFontFamily;
};

#endif // MARKDOWNTHEME_H
