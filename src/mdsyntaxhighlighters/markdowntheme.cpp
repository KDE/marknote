// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "markdowntheme.h"

#include <QFontDatabase>

MarkdownTheme::MarkdownTheme()
{
}

QColor MarkdownTheme::textColor() const
{
    return m_textColor;
}

void MarkdownTheme::setTextColor(const QColor &color)
{
    m_textColor = color;
}

QColor MarkdownTheme::dimmedColor() const
{
    return m_dimmedColor;
}

void MarkdownTheme::setDimmedColor(const QColor &color)
{
    m_dimmedColor = color;
}

QColor MarkdownTheme::codeColor() const
{
    return m_codeColor;
}

void MarkdownTheme::setCodeColor(const QColor &color)
{
    m_codeColor = color;
}

QString MarkdownTheme::fixedFontFamily() const
{
    return m_fixedFontFamily;
}

void MarkdownTheme::setFixedFontFamily(const QString &family)
{
    m_fixedFontFamily = family;
}

QTextCharFormat MarkdownTheme::formatForInline(MDOptions::InlineStyle type) const
{
    QTextCharFormat format;

    switch (type) {
    case MDOptions::InlineStyle::Emphasis:
        format.setFontItalic(true);
        break;
    case MDOptions::InlineStyle::Strong:
        format.setFontWeight(QFont::Bold);
        break;
    case MDOptions::InlineStyle::Strikethrough:
        format.setFontStrikeOut(true);
        break;
    case MDOptions::InlineStyle::CodeSpan:
        if (!m_fixedFontFamily.isEmpty()) {
            format.setFontFamilies({m_fixedFontFamily});
        } else {
            format.setFontFamilies({QFontDatabase::systemFont(QFontDatabase::FixedFont).family()});
        }
        if (m_codeColor.isValid()) {
            format.setForeground(m_codeColor);
        }
        break;
    default:
        break;
    }

    return format;
}

QTextCharFormat MarkdownTheme::delimiterFormat() const
{
    QTextCharFormat format;
    if (m_dimmedColor.isValid()) {
        format.setForeground(m_dimmedColor);
    }
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    format.setFontStrikeOut(false);
    return format;
}

QTextCharFormat MarkdownTheme::formatForHeading(int level, qreal basePointSize) const
{
    QTextCharFormat format;
    qreal multiplier = 1.0;
    switch (level) {
    case 1:
        multiplier = 2.0;
        break;
    case 2:
        multiplier = 1.5;
        break;
    case 3:
        multiplier = 1.2;
        break;
    case 4:
        multiplier = 1.0;
        break;
    case 5:
        multiplier = 0.8;
        break;
    case 6:
        multiplier = 1.0;
        break;
    default:
        multiplier = 1.0;
        break;
    }

    if (basePointSize > 0) {
        format.setFontPointSize(basePointSize * multiplier);
    }
    format.setFontWeight(QFont::Bold);
    if (m_textColor.isValid()) {
        format.setForeground(m_textColor);
    }
    return format;
}

QTextCharFormat MarkdownTheme::formatForBlockquote() const
{
    QTextCharFormat format;
    format.setFontItalic(true);
    return format;
}

QTextCharFormat MarkdownTheme::formatForCodeBlock() const
{
    QTextCharFormat format;
    if (!m_fixedFontFamily.isEmpty()) {
        format.setFontFamilies({m_fixedFontFamily});
    } else {
        format.setFontFamilies({QFontDatabase::systemFont(QFontDatabase::FixedFont).family()});
    }
    if (m_codeColor.isValid()) {
        format.setForeground(m_codeColor);
    }
    return format;
}

QTextCharFormat MarkdownTheme::formatForLink() const
{
    QTextCharFormat format;
    format.setFontUnderline(true);
    if (m_textColor.isValid()) {
        format.setForeground(m_textColor);
    }
    return format;
}
