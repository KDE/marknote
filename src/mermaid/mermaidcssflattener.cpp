// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidcssflattener.h"
#include <marknotesettings.h>

#include <KColorScheme>

#include <QColor>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QMap>
#include <QPalette>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Qt::StringLiterals;

namespace
{
qreal parseSvgLength(QStringView raw)
{
    QStringView trimmed = raw.trimmed();
    qreal scale = 1.0;
    if (trimmed.endsWith(u"px"_s, Qt::CaseInsensitive)) {
        trimmed = trimmed.left(trimmed.size() - 2).trimmed();
    } else if (trimmed.endsWith(u"pt"_s, Qt::CaseInsensitive)) {
        trimmed = trimmed.left(trimmed.size() - 2).trimmed();
        scale = 96.0 / 72.0;
    }

    bool ok = false;
    const qreal val = trimmed.toDouble(&ok);
    return ok ? (val * scale) : 0.0;
}

void resolveCssVariables(QString &svg, const QHash<QString, QString> &vars)
{
    QHash<QString, QString> aliases;
    aliases[u"--_bg"_s] = vars.value(u"--pele-bg"_s);
    aliases[u"--_fg"_s] = vars.value(u"--pele-fg"_s);
    aliases[u"--_m"_s] = vars.value(u"--pele-muted"_s);
    aliases[u"--_l"_s] = vars.value(u"--pele-line"_s);
    aliases[u"--_s"_s] = vars.value(u"--pele-surface"_s);
    aliases[u"--_a"_s] = vars.value(u"--pele-surface-alt"_s);
    aliases[u"--_b"_s] = vars.value(u"--pele-border"_s);
    aliases[u"--_c"_s] = vars.value(u"--pele-accent"_s);
    aliases[u"--_r"_s] = vars.value(u"--pele-radius"_s, u"4px"_s);
    aliases[u"--_w"_s] = vars.value(u"--pele-bold-weight"_s, u"bold"_s);
    aliases[u"--_hw"_s] = vars.value(u"--pele-heading-weight"_s, u"bold"_s);
    aliases[u"--_tw"_s] = vars.value(u"--pele-title-weight"_s, u"bold"_s);
    aliases[u"--_fm"_s] = vars.value(u"--pele-font-mono"_s, u"monospace"_s);

    for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
        svg.replace(u"var(%1)"_s.arg(it.key()), it.value());
    }

    constexpr QLatin1StringView prefix("var(--pele-");
    qsizetype pos = 0;
    while ((pos = svg.indexOf(prefix, pos)) != -1) {
        const qsizetype nameStart = pos + 4; // starts at "--pele-"
        qsizetype cursor = pos + prefix.size();
        while (cursor < svg.size()) {
            const QChar ch = svg.at(cursor);
            if (!ch.isLetterOrNumber() && ch != QLatin1Char('_') && ch != QLatin1Char('-')) {
                break;
            }
            ++cursor;
        }
        const QString varName = svg.mid(nameStart, cursor - nameStart);

        QString fallback;
        bool validCall = false;
        qsizetype endPos = cursor;
        if (cursor < svg.size() && svg.at(cursor) == QLatin1Char(')')) {
            validCall = true;
            endPos = cursor + 1;
        } else if (cursor < svg.size() && svg.at(cursor) == QLatin1Char(',')) {
            int depth = 1;
            const qsizetype fbStart = cursor + 1;
            ++cursor;
            while (cursor < svg.size() && depth > 0) {
                const QChar ch = svg.at(cursor);
                if (ch == QLatin1Char('(')) {
                    ++depth;
                } else if (ch == QLatin1Char(')')) {
                    --depth;
                }
                ++cursor;
            }
            if (depth == 0) {
                validCall = true;
                endPos = cursor;
                fallback = svg.mid(fbStart, (cursor - 1) - fbStart).trimmed();
            }
        }

        if (!validCall) {
            pos += prefix.size();
            continue;
        }

        const QString replacement = vars.value(varName, fallback.isEmpty() ? u"none"_s : fallback);
        svg.replace(pos, endPos - pos, replacement);
        pos += replacement.size();
    }
}

QString normalizeSvgStructure(const QString &svg, const QString &defaultFgColor)
{
    const qsizetype firstSvg = svg.indexOf(u"<svg"_s);
    if (firstSvg == -1) {
        return svg;
    }

    const bool hasNestedSvg = svg.indexOf(u"<svg"_s, firstSvg + 4) != -1;
    const bool hasStyleAttr = svg.contains(u"style="_s);
    const bool hasCurrentColor = svg.contains(u"currentColor"_s);
    if (!hasNestedSvg && !hasStyleAttr && !hasCurrentColor) {
        return svg;
    }

    QString out;
    out.reserve(svg.size());

    QXmlStreamReader reader(svg);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(false);

    int svgDepth = 0;
    QStringList colorStack = {defaultFgColor};

    auto pushScopeColor = [&colorStack](const QXmlStreamAttributes &attrs, const QString &styleOverride) {
        QString nextColor = colorStack.last();
        const QString colorAttr = attrs.value(u"color"_s).toString().trimmed();
        if (!colorAttr.isEmpty() && colorAttr != u"currentColor"_s) {
            nextColor = colorAttr;
        }
        const QString styleStr = styleOverride.isNull() ? attrs.value(u"style"_s).toString() : styleOverride;
        if (!styleStr.isEmpty()) {
            const QStringList decls = styleStr.split(QLatin1Char(';'), Qt::SkipEmptyParts);
            for (const QString &rawDecl : decls) {
                const QString decl = rawDecl.trimmed();
                const qsizetype colon = decl.indexOf(QLatin1Char(':'));
                if (colon > 0 && decl.left(colon).trimmed() == u"color"_s) {
                    const QString val = decl.mid(colon + 1).trimmed();
                    if (!val.isEmpty() && val != u"currentColor"_s) {
                        nextColor = val;
                    }
                }
            }
        }
        colorStack.append(nextColor);
    };

    auto resolveColorVal = [&colorStack](QString val) {
        if (val.contains(u"currentColor"_s)) {
            val.replace(u"currentColor"_s, colorStack.last());
        }
        return val;
    };

    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement: {
            if (reader.name() == u"svg"_s) {
                if (svgDepth == 0) {
                    for (const auto &ns : reader.namespaceDeclarations()) {
                        if (ns.prefix().isEmpty()) {
                            writer.writeDefaultNamespace(ns.namespaceUri().toString());
                        } else {
                            writer.writeNamespace(ns.namespaceUri().toString(), ns.prefix().toString());
                        }
                    }
                    writer.writeStartElement(reader.qualifiedName().toString());

                    const QXmlStreamAttributes attrs = reader.attributes();
                    QString fontSizeAttr;
                    QString fontFamilyAttr;
                    QString cleanedStyle;
                    bool hasExplicitFontSize = false;
                    bool hasExplicitFontFamily = false;

                    for (const auto &attr : attrs) {
                        if (attr.name() == u"font-size"_s) {
                            hasExplicitFontSize = true;
                        } else if (attr.name() == u"font-family"_s) {
                            hasExplicitFontFamily = true;
                        }
                    }

                    // Pre-extract cleaned style so colorStack uses resolved root color
                    const QString rawStyle = attrs.value(u"style"_s).toString();
                    if (!rawStyle.isEmpty()) {
                        const QStringList decls = rawStyle.split(QLatin1Char(';'), Qt::SkipEmptyParts);
                        QStringList keptDecls;
                        for (const QString &rawDecl : decls) {
                            const QString decl = rawDecl.trimmed();
                            const qsizetype colon = decl.indexOf(QLatin1Char(':'));
                            if (colon <= 0) {
                                continue;
                            }
                            const QString prop = decl.left(colon).trimmed();
                            const QString val = decl.mid(colon + 1).trimmed();
                            if (prop.startsWith(u"--"_s) || prop == u"max-width"_s || prop == u"height"_s || prop == u"letter-spacing"_s
                                || prop == u"word-spacing"_s || prop == u"text-transform"_s || prop == u"direction"_s) {
                                continue;
                            }
                            if (prop == u"font"_s) {
                                static const QRegularExpression fontShorthandRegex(u"^([0-9]+(?:\\.[0-9]+)?)px\\b"_s);
                                const auto fm = fontShorthandRegex.match(val);
                                if (fm.hasMatch() && !hasExplicitFontSize) {
                                    fontSizeAttr = fm.captured(1);
                                }
                                continue;
                            }
                            if (prop == u"font-family"_s) {
                                if (!hasExplicitFontFamily && !val.isEmpty()) {
                                    fontFamilyAttr = val;
                                }
                                continue;
                            }
                            if (prop == u"font-size"_s) {
                                if (!hasExplicitFontSize && !val.isEmpty()) {
                                    QString sz = val;
                                    sz.remove(u"px"_s);
                                    fontSizeAttr = sz;
                                }
                                continue;
                            }
                            keptDecls.append(prop + u":"_s + val);
                        }
                        cleanedStyle = keptDecls.join(QLatin1Char(';'));
                    }

                    pushScopeColor(attrs, cleanedStyle);

                    for (const auto &attr : attrs) {
                        if (attr.name() == u"style"_s) {
                            if (!cleanedStyle.isEmpty()) {
                                writer.writeAttribute(u"style"_s, resolveColorVal(cleanedStyle));
                            }
                        } else {
                            writer.writeAttribute(attr.qualifiedName().toString(), resolveColorVal(attr.value().toString()));
                        }
                    }

                    if (!fontSizeAttr.isEmpty()) {
                        writer.writeAttribute(u"font-size"_s, fontSizeAttr);
                    }
                    if (!fontFamilyAttr.isEmpty()) {
                        writer.writeAttribute(u"font-family"_s, fontFamilyAttr);
                    }
                } else {
                    const QXmlStreamAttributes attrs = reader.attributes();
                    pushScopeColor(attrs, QString());
                    qreal x = 0.0;
                    qreal y = 0.0;
                    qreal w = 0.0;
                    qreal h = 0.0;
                    QString viewBoxStr;
                    QString existingTransform;

                    for (const auto &attr : attrs) {
                        const auto name = attr.name();
                        if (name == u"x"_s) {
                            x = parseSvgLength(attr.value());
                        } else if (name == u"y"_s) {
                            y = parseSvgLength(attr.value());
                        } else if (name == u"width"_s) {
                            w = parseSvgLength(attr.value());
                        } else if (name == u"height"_s) {
                            h = parseSvgLength(attr.value());
                        } else if (name == u"viewBox"_s) {
                            viewBoxStr = attr.value().toString();
                        } else if (name == u"transform"_s) {
                            existingTransform = attr.value().toString();
                        }
                    }

                    QStringList transforms;
                    if (!existingTransform.isEmpty()) {
                        transforms.append(existingTransform);
                    }
                    if (!qFuzzyIsNull(x) || !qFuzzyIsNull(y)) {
                        transforms.append(u"translate(%1, %2)"_s.arg(x).arg(y));
                    }
                    if (!viewBoxStr.isEmpty()) {
                        static const QRegularExpression vbSplitRegex(u"\\s+|,"_s);
                        const QStringList vbParts = viewBoxStr.trimmed().split(vbSplitRegex, Qt::SkipEmptyParts);
                        if (vbParts.size() == 4) {
                            const qreal minX = vbParts[0].toDouble();
                            const qreal minY = vbParts[1].toDouble();
                            const qreal vbW = vbParts[2].toDouble();
                            const qreal vbH = vbParts[3].toDouble();
                            if (vbW > 0.0 && vbH > 0.0) {
                                const qreal sx = w > 0.0 ? (w / vbW) : 1.0;
                                const qreal sy = h > 0.0 ? (h / vbH) : 1.0;
                                if (!qFuzzyCompare(sx, 1.0) || !qFuzzyCompare(sy, 1.0)) {
                                    transforms.append(u"scale(%1, %2)"_s.arg(sx).arg(sy));
                                }
                                if (!qFuzzyIsNull(minX) || !qFuzzyIsNull(minY)) {
                                    transforms.append(u"translate(%1, %2)"_s.arg(-minX).arg(-minY));
                                }
                            }
                        }
                    }

                    writer.writeStartElement(u"g"_s);
                    for (const auto &attr : attrs) {
                        const auto name = attr.name();
                        if (name == u"x"_s || name == u"y"_s || name == u"width"_s || name == u"height"_s || name == u"viewBox"_s || name == u"overflow"_s
                            || name == u"transform"_s) {
                            continue;
                        }
                        writer.writeAttribute(attr.qualifiedName().toString(), resolveColorVal(attr.value().toString()));
                    }
                    if (!transforms.isEmpty()) {
                        writer.writeAttribute(u"transform"_s, transforms.join(QLatin1Char(' ')));
                    }
                }
                ++svgDepth;
            } else {
                const QXmlStreamAttributes attrs = reader.attributes();
                pushScopeColor(attrs, QString());
                for (const auto &ns : reader.namespaceDeclarations()) {
                    if (ns.prefix().isEmpty()) {
                        writer.writeDefaultNamespace(ns.namespaceUri().toString());
                    } else {
                        writer.writeNamespace(ns.namespaceUri().toString(), ns.prefix().toString());
                    }
                }
                writer.writeStartElement(reader.qualifiedName().toString());

                QString promotedRx;
                QString cleanedStyle;
                const QString rawStyle = attrs.value(u"style"_s).toString();
                const bool hasExplicitRx = attrs.hasAttribute(u"rx"_s);
                if (!rawStyle.isEmpty() && rawStyle.contains(u"rx:"_s)) {
                    const QStringList decls = rawStyle.split(QLatin1Char(';'), Qt::SkipEmptyParts);
                    QStringList keptDecls;
                    for (const QString &rawDecl : decls) {
                        const QString decl = rawDecl.trimmed();
                        const qsizetype colon = decl.indexOf(QLatin1Char(':'));
                        if (colon > 0 && decl.left(colon).trimmed() == u"rx"_s) {
                            if (!hasExplicitRx) {
                                promotedRx = decl.mid(colon + 1).trimmed();
                                promotedRx.remove(u"px"_s);
                            }
                            continue;
                        }
                        keptDecls.append(decl);
                    }
                    cleanedStyle = keptDecls.join(QLatin1Char(';'));
                }

                for (const auto &attr : attrs) {
                    if (attr.name() == u"style"_s && !rawStyle.isEmpty() && rawStyle.contains(u"rx:"_s)) {
                        if (!cleanedStyle.isEmpty()) {
                            writer.writeAttribute(u"style"_s, resolveColorVal(cleanedStyle));
                        }
                    } else {
                        writer.writeAttribute(attr.qualifiedName().toString(), resolveColorVal(attr.value().toString()));
                    }
                }
                if (!promotedRx.isEmpty()) {
                    writer.writeAttribute(u"rx"_s, promotedRx);
                }
            }
            break;
        }
        case QXmlStreamReader::EndElement:
            if (reader.name() == u"svg"_s) {
                --svgDepth;
            }
            if (colorStack.size() > 1) {
                colorStack.removeLast();
            }
            writer.writeEndElement();
            break;
        case QXmlStreamReader::Characters:
            if (reader.isCDATA()) {
                writer.writeCDATA(reader.text().toString());
            } else {
                writer.writeCharacters(reader.text().toString());
            }
            break;
        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;
        default:
            break;
        }
    }

    if (reader.hasError()) {
        return svg;
    }

    return out;
}

struct XmlNodeToken {
    enum Kind {
        Start,
        End,
        Chars,
        Comment,
    };
    Kind kind = Chars;
    QString name;
    QList<QPair<QString, QString>> attrs;
    QString text;
    bool isCdata = false;
};

void writeTokens(QXmlStreamWriter &writer, const QList<XmlNodeToken> &tokens)
{
    for (const XmlNodeToken &tok : tokens) {
        switch (tok.kind) {
        case XmlNodeToken::Start:
            writer.writeStartElement(tok.name);
            for (const auto &attr : tok.attrs) {
                writer.writeAttribute(attr.first, attr.second);
            }
            break;
        case XmlNodeToken::End:
            writer.writeEndElement();
            break;
        case XmlNodeToken::Chars:
            if (tok.isCdata) {
                writer.writeCDATA(tok.text);
            } else {
                writer.writeCharacters(tok.text);
            }
            break;
        case XmlNodeToken::Comment:
            writer.writeComment(tok.text);
            break;
        }
    }
}
} // namespace

QHash<QString, QString> MermaidCssFlattener::defaultVariables(bool isDark)
{
    QHash<QString, QString> vars;

    const bool appAvailable = (qApp != nullptr);
    const bool appIsDark = appAvailable && (qApp->palette().color(QPalette::Window).lightness() < 128);

    if (appAvailable && (appIsDark == isDark)) {
        KColorScheme viewScheme(QPalette::Active, KColorScheme::View);
        KColorScheme windowScheme(QPalette::Active, KColorScheme::Window);
        KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button);
        KColorScheme selScheme(QPalette::Active, KColorScheme::Selection);

        auto toHex = [](const QColor &c) -> QString {
            return c.name(QColor::HexRgb);
        };

        vars[u"--pele-bg"_s] = toHex(viewScheme.background(KColorScheme::NormalBackground).color());
        vars[u"--pele-fg"_s] = toHex(viewScheme.foreground(KColorScheme::NormalText).color());
        vars[u"--pele-fg-on-light"_s] = u"#232629"_s;
        vars[u"--pele-fg-on-dark"_s] = u"#eff0f1"_s;
        vars[u"--pele-muted"_s] = toHex(viewScheme.foreground(KColorScheme::InactiveText).color());
        vars[u"--pele-line"_s] = toHex(viewScheme.foreground(KColorScheme::NormalText).color());
        vars[u"--pele-surface"_s] = toHex(windowScheme.background(KColorScheme::NormalBackground).color());
        vars[u"--pele-surface-alt"_s] = toHex(windowScheme.background(KColorScheme::AlternateBackground).color());
        vars[u"--pele-border"_s] = toHex(buttonScheme.shade(KColorScheme::MidShade));
        vars[u"--pele-accent"_s] = toHex(selScheme.background(KColorScheme::NormalBackground).color());
        vars[u"--pele-critical"_s] = toHex(viewScheme.foreground(KColorScheme::NegativeText).color());
        vars[u"--pele-error"_s] = toHex(viewScheme.foreground(KColorScheme::NegativeText).color());
    } else if (isDark) {
        vars[u"--pele-bg"_s] = u"#232629"_s;
        vars[u"--pele-fg"_s] = u"#eff0f1"_s;
        vars[u"--pele-fg-on-light"_s] = u"#232629"_s;
        vars[u"--pele-fg-on-dark"_s] = u"#eff0f1"_s;
        vars[u"--pele-muted"_s] = u"#bdc3c7"_s;
        vars[u"--pele-line"_s] = u"#eff0f1"_s;
        vars[u"--pele-surface"_s] = u"#31363b"_s;
        vars[u"--pele-surface-alt"_s] = u"#2a2e32"_s;
        vars[u"--pele-border"_s] = u"#4d545b"_s;
        vars[u"--pele-accent"_s] = u"#3daee9"_s;
        vars[u"--pele-critical"_s] = u"#da4453"_s;
        vars[u"--pele-error"_s] = u"#da4453"_s;
    } else {
        vars[u"--pele-bg"_s] = u"#ffffff"_s;
        vars[u"--pele-fg"_s] = u"#232629"_s;
        vars[u"--pele-fg-on-light"_s] = u"#232629"_s;
        vars[u"--pele-fg-on-dark"_s] = u"#eff0f1"_s;
        vars[u"--pele-muted"_s] = u"#7f8c8d"_s;
        vars[u"--pele-line"_s] = u"#232629"_s;
        vars[u"--pele-surface"_s] = u"#f7f7f7"_s;
        vars[u"--pele-surface-alt"_s] = u"#eff0f1"_s;
        vars[u"--pele-border"_s] = u"#bdc3c7"_s;
        vars[u"--pele-accent"_s] = u"#3daee9"_s;
        vars[u"--pele-critical"_s] = u"#da4453"_s;
        vars[u"--pele-error"_s] = u"#da4453"_s;
    }

    vars[u"--pele-radius"_s] = u"4px"_s;
    vars[u"--pele-bold-weight"_s] = u"bold"_s;
    vars[u"--pele-heading-weight"_s] = u"bold"_s;
    vars[u"--pele-title-weight"_s] = u"bold"_s;

    const QFont font = Config::editorFont();
    const QString family = font.family().isEmpty() ? QFontDatabase::systemFont(QFontDatabase::GeneralFont).family() : font.family();
    vars[u"--pele-font"_s] = family.isEmpty() ? u"sans-serif"_s : family;
    vars[u"--pele-font-mono"_s] =
        QFontDatabase::systemFont(QFontDatabase::FixedFont).family().isEmpty() ? u"monospace"_s : QFontDatabase::systemFont(QFontDatabase::FixedFont).family();

    vars[u"--pele-series-1"_s] = vars.value(u"--pele-accent"_s, u"#3daee9"_s);
    vars[u"--pele-series-2"_s] = u"#2ecc71"_s;
    vars[u"--pele-series-3"_s] = u"#f67400"_s;
    vars[u"--pele-series-4"_s] = u"#e74c3c"_s;
    vars[u"--pele-series-5"_s] = u"#9b59b6"_s;
    vars[u"--pele-series-6"_s] = u"#1abc9c"_s;
    vars[u"--pele-series-7"_s] = u"#f1c40f"_s;
    vars[u"--pele-series-8"_s] = u"#e67e22"_s;

    return vars;
}

QString MermaidCssFlattener::flattenSvgResolved(const QString &rawSvg, const QHash<QString, QString> &resolvedVariables)
{
    QString svg = rawSvg;
    resolveCssVariables(svg, resolvedVariables);
    svg = normalizeSvgStructure(svg, resolvedVariables.value(u"--pele-fg"_s));
    svg = normalizeTspans(svg);
    return svg;
}

QString MermaidCssFlattener::flattenSvg(const QString &rawSvg, const QHash<QString, QString> &customVariables, bool isDark)
{
    QHash<QString, QString> vars = defaultVariables(isDark);
    for (auto it = customVariables.constBegin(); it != customVariables.constEnd(); ++it) {
        vars[it.key()] = it.value();
    }
    return flattenSvgResolved(rawSvg, vars);
}

QString MermaidCssFlattener::normalizeTspans(const QString &svg)
{
    if (!svg.contains(u"<tspan"_s)) {
        return svg;
    }

    QString cleanSvg;
    cleanSvg.reserve(svg.size());
    for (int i = 0; i < svg.size(); ++i) {
        const QChar c = svg.at(i);
        if (c.isHighSurrogate()) {
            if (i + 1 < svg.size() && svg.at(i + 1).isLowSurrogate()) {
                cleanSvg.append(c);
                cleanSvg.append(svg.at(++i));
            }
        } else if (!c.isLowSurrogate()) {
            cleanSvg.append(c);
        }
    }

    const bool hasXmlDecl = cleanSvg.trimmed().startsWith(u"<?"_s);
    const QString parseInput = hasXmlDecl ? cleanSvg : (u"<_root>"_s + cleanSvg + u"</_root>"_s);

    QString out;
    out.reserve(cleanSvg.size());

    QXmlStreamReader reader(parseInput);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(false);

    auto copyAttributes = [](const QXmlStreamAttributes &attrs) {
        QList<QPair<QString, QString>> list;
        list.reserve(attrs.size());
        for (const auto &attr : attrs) {
            list.append({attr.qualifiedName().toString(), attr.value().toString()});
        }
        return list;
    };

    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement: {
            if (!hasXmlDecl && reader.name() == u"_root"_s) {
                break;
            }
            if (reader.name() != u"text"_s) {
                for (const auto &ns : reader.namespaceDeclarations()) {
                    if (ns.prefix().isEmpty()) {
                        writer.writeDefaultNamespace(ns.namespaceUri().toString());
                    } else {
                        writer.writeNamespace(ns.namespaceUri().toString(), ns.prefix().toString());
                    }
                }
                writer.writeStartElement(reader.qualifiedName().toString());
                for (const auto &attr : reader.attributes()) {
                    writer.writeAttribute(attr.qualifiedName().toString(), attr.value().toString());
                }
                break;
            }

            QMap<QString, QString> baseAttrs;
            QList<QPair<QString, QString>> rawBaseAttrOrder;
            for (const auto &attr : reader.attributes()) {
                const QString qname = attr.qualifiedName().toString();
                const QString val = attr.value().toString();
                baseAttrs.insert(qname, val);
                rawBaseAttrOrder.append({qname, val});
            }

            struct SpanInfo {
                QMap<QString, QString> attrs;
                QList<XmlNodeToken> bodyTokens;
            };
            struct LineInfo {
                QMap<QString, QString> textAttrs;
                QList<SpanInfo> spans;
            };

            QList<XmlNodeToken> originalChildren;
            QList<LineInfo> lines;
            LineInfo currentLine;
            currentLine.textAttrs = baseAttrs;
            bool hasPositionedTspan = false;
            bool hasAnyTspan = false;

            int textDepth = 1;
            while (!reader.atEnd() && textDepth > 0) {
                const auto innerTok = reader.readNext();
                if (innerTok == QXmlStreamReader::StartElement) {
                    if (reader.name() == u"tspan"_s && textDepth == 1) {
                        hasAnyTspan = true;
                        QMap<QString, QString> tattrs;
                        for (const auto &attr : reader.attributes()) {
                            tattrs.insert(attr.qualifiedName().toString(), attr.value().toString());
                        }
                        originalChildren.append({XmlNodeToken::Start, reader.qualifiedName().toString(), copyAttributes(reader.attributes()), {}, false});

                        QList<XmlNodeToken> spanBody;
                        int tspanDepth = 1;
                        while (!reader.atEnd() && tspanDepth > 0) {
                            const auto st = reader.readNext();
                            if (st == QXmlStreamReader::StartElement) {
                                ++tspanDepth;
                                XmlNodeToken node{XmlNodeToken::Start, reader.qualifiedName().toString(), copyAttributes(reader.attributes()), {}, false};
                                spanBody.append(node);
                                originalChildren.append(node);
                            } else if (st == QXmlStreamReader::EndElement) {
                                --tspanDepth;
                                if (tspanDepth > 0) {
                                    XmlNodeToken node{XmlNodeToken::End, reader.qualifiedName().toString(), {}, {}, false};
                                    spanBody.append(node);
                                }
                                originalChildren.append({XmlNodeToken::End, reader.qualifiedName().toString(), {}, {}, false});
                            } else if (st == QXmlStreamReader::Characters) {
                                XmlNodeToken node{XmlNodeToken::Chars, {}, {}, reader.text().toString(), reader.isCDATA()};
                                spanBody.append(node);
                                originalChildren.append(node);
                            } else if (st == QXmlStreamReader::Comment) {
                                XmlNodeToken node{XmlNodeToken::Comment, {}, {}, reader.text().toString(), false};
                                spanBody.append(node);
                                originalChildren.append(node);
                            }
                        }

                        if (tattrs.contains(u"y"_s) || tattrs.contains(u"dy"_s)) {
                            hasPositionedTspan = true;
                            if (!currentLine.spans.isEmpty()) {
                                lines.append(currentLine);
                                currentLine.spans.clear();
                            }
                            currentLine.textAttrs = baseAttrs;
                            if (tattrs.contains(u"x"_s)) {
                                currentLine.textAttrs.insert(u"x"_s, tattrs.value(u"x"_s));
                            }
                            if (tattrs.contains(u"y"_s)) {
                                currentLine.textAttrs.insert(u"y"_s, tattrs.value(u"y"_s));
                            }
                            if (tattrs.contains(u"dy"_s)) {
                                currentLine.textAttrs.insert(u"dy"_s, tattrs.value(u"dy"_s));
                            }
                        }

                        QMap<QString, QString> innerAttrs;
                        for (auto ait = tattrs.constBegin(); ait != tattrs.constEnd(); ++ait) {
                            if (ait.key() != u"x"_s && ait.key() != u"y"_s && ait.key() != u"dy"_s) {
                                innerAttrs.insert(ait.key(), ait.value());
                            }
                        }
                        currentLine.spans.append({innerAttrs, spanBody});
                    } else {
                        ++textDepth;
                        XmlNodeToken node{XmlNodeToken::Start, reader.qualifiedName().toString(), copyAttributes(reader.attributes()), {}, false};
                        originalChildren.append(node);
                        if (currentLine.spans.isEmpty() || !currentLine.spans.last().attrs.isEmpty()) {
                            currentLine.spans.append({{}, {node}});
                        } else {
                            currentLine.spans.last().bodyTokens.append(node);
                        }
                    }
                } else if (innerTok == QXmlStreamReader::EndElement) {
                    --textDepth;
                    if (textDepth > 0) {
                        XmlNodeToken node{XmlNodeToken::End, reader.qualifiedName().toString(), {}, {}, false};
                        originalChildren.append(node);
                        if (!currentLine.spans.isEmpty()) {
                            currentLine.spans.last().bodyTokens.append(node);
                        }
                    }
                } else if (innerTok == QXmlStreamReader::Characters) {
                    XmlNodeToken node{XmlNodeToken::Chars, {}, {}, reader.text().toString(), reader.isCDATA()};
                    originalChildren.append(node);
                    currentLine.spans.append({{}, {node}});
                } else if (innerTok == QXmlStreamReader::Comment) {
                    XmlNodeToken node{XmlNodeToken::Comment, {}, {}, reader.text().toString(), false};
                    originalChildren.append(node);
                    currentLine.spans.append({{}, {node}});
                }
            }

            if (!currentLine.spans.isEmpty()) {
                lines.append(currentLine);
            }

            if (!hasAnyTspan || !hasPositionedTspan || lines.isEmpty()) {
                writer.writeStartElement(u"text"_s);
                for (const auto &attr : rawBaseAttrOrder) {
                    writer.writeAttribute(attr.first, attr.second);
                }
                writeTokens(writer, originalChildren);
                writer.writeEndElement();
            } else {
                for (const auto &line : lines) {
                    QMap<QString, QString> finalAttrs = line.textAttrs;
                    if (line.spans.size() == 1) {
                        const auto &span = line.spans.first();
                        for (auto sait = span.attrs.constBegin(); sait != span.attrs.constEnd(); ++sait) {
                            finalAttrs.insert(sait.key(), sait.value());
                        }
                        writer.writeStartElement(u"text"_s);
                        for (auto ait = finalAttrs.constBegin(); ait != finalAttrs.constEnd(); ++ait) {
                            writer.writeAttribute(ait.key(), ait.value());
                        }
                        writeTokens(writer, span.bodyTokens);
                        writer.writeEndElement();
                    } else {
                        if (!finalAttrs.contains(u"xml:space"_s)) {
                            finalAttrs.insert(u"xml:space"_s, u"preserve"_s);
                        }
                        writer.writeStartElement(u"text"_s);
                        for (auto ait = finalAttrs.constBegin(); ait != finalAttrs.constEnd(); ++ait) {
                            writer.writeAttribute(ait.key(), ait.value());
                        }
                        for (const auto &span : line.spans) {
                            writer.writeStartElement(u"tspan"_s);
                            for (auto sait = span.attrs.constBegin(); sait != span.attrs.constEnd(); ++sait) {
                                writer.writeAttribute(sait.key(), sait.value());
                            }
                            writeTokens(writer, span.bodyTokens);
                            writer.writeEndElement();
                        }
                        writer.writeEndElement();
                    }
                }
            }
            break;
        }
        case QXmlStreamReader::EndElement:
            if (!hasXmlDecl && reader.name() == u"_root"_s) {
                break;
            }
            writer.writeEndElement();
            break;
        case QXmlStreamReader::Characters:
            if (reader.isCDATA()) {
                writer.writeCDATA(reader.text().toString());
            } else {
                writer.writeCharacters(reader.text().toString());
            }
            break;
        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;
        default:
            break;
        }
    }

    if (reader.hasError()) {
        return svg;
    }

    return out;
}
