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

using namespace Qt::StringLiterals;

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

QString MermaidCssFlattener::flattenSvg(const QString &rawSvg, const QHash<QString, QString> &customVariables, bool isDark)
{
    QString svg = rawSvg;

    QHash<QString, QString> vars = defaultVariables(isDark);
    for (auto it = customVariables.constBegin(); it != customVariables.constEnd(); ++it) {
        vars[it.key()] = it.value();
    }

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

    static const QRegularExpression peleVarRegex(u"var\\((--pele-[a-zA-Z0-9_-]+)(?:,([^)]+))?\\)"_s);
    int offset = 0;
    QRegularExpressionMatch match;
    while ((match = peleVarRegex.match(svg, offset)).hasMatch()) {
        QString varName = match.captured(1);
        QString fallback = match.captured(2);
        QString replacement = vars.value(varName, fallback.isEmpty() ? u"none"_s : fallback);
        svg.replace(match.capturedStart(), match.capturedLength(), replacement);
        offset = match.capturedStart() + replacement.length();
    }

    svg.replace(u"currentColor"_s, vars.value(u"--pele-fg"_s));

    static const QRegularExpression rxStyleRegex(u"style=\"rx:([^;\"]+)(?:;rx:[^;\"]+)*\""_s);
    QString radiusVal = vars.value(u"--pele-radius"_s, u"4px"_s);
    radiusVal.remove(u"px"_s);
    svg.replace(rxStyleRegex, u"rx=\"%1\""_s.arg(radiusVal));

    svg = normalizeTspans(svg);

    return svg;
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

    static const QRegularExpression textRegex(u"<text\\b([^>]*)>([\\s\\S]*?)</text>"_s);
    static const QRegularExpression tspanWithYRegex(u"<tspan\\b[^>]*\\b(?:y|dy)="_s);
    static const QRegularExpression tspanRegex(u"<tspan\\b([^>]*)>([\\s\\S]*?)</tspan>"_s);
    static const QRegularExpression attrRegex(u"([a-zA-Z0-9:_-]+)=\"([^\"]*)\""_s);

    QString result;
    result.reserve(cleanSvg.size());
    int lastPos = 0;
    auto it = textRegex.globalMatch(cleanSvg);

    while (it.hasNext()) {
        auto match = it.next();
        result.append(QStringView(cleanSvg).mid(lastPos, match.capturedStart() - lastPos));
        lastPos = match.capturedEnd();

        const QString textAttrs = match.captured(1);
        const QString content = match.captured(2);

        if (!tspanWithYRegex.match(content).hasMatch()) {
            result.append(match.captured(0));
            continue;
        }

        QMap<QString, QString> baseAttrs;
        auto attrIt = attrRegex.globalMatch(textAttrs);
        while (attrIt.hasNext()) {
            auto am = attrIt.next();
            baseAttrs.insert(am.captured(1), am.captured(2));
        }

        struct SpanInfo {
            QMap<QString, QString> attrs;
            QString body;
        };

        struct LineInfo {
            QMap<QString, QString> textAttrs;
            QList<SpanInfo> spans;
        };

        QList<LineInfo> lines;
        LineInfo currentLine;
        currentLine.textAttrs = baseAttrs;
        bool hasTspans = false;

        auto tspanIt = tspanRegex.globalMatch(content);
        while (tspanIt.hasNext()) {
            hasTspans = true;
            auto tm = tspanIt.next();
            const QString tspanAttrs = tm.captured(1);
            const QString tspanBody = tm.captured(2);

            QMap<QString, QString> tattrs;
            auto amIt = attrRegex.globalMatch(tspanAttrs);
            while (amIt.hasNext()) {
                auto am = amIt.next();
                tattrs.insert(am.captured(1), am.captured(2));
            }

            if (tattrs.contains(u"y"_s) || tattrs.contains(u"dy"_s)) {
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

            currentLine.spans.append({innerAttrs, tspanBody});
        }

        if (!currentLine.spans.isEmpty()) {
            lines.append(currentLine);
        }

        if (!hasTspans || lines.isEmpty()) {
            result.append(match.captured(0));
        } else {
            for (const auto &line : lines) {
                QMap<QString, QString> finalAttrs = line.textAttrs;
                if (line.spans.size() == 1) {
                    const auto &span = line.spans.first();
                    for (auto sait = span.attrs.constBegin(); sait != span.attrs.constEnd(); ++sait) {
                        finalAttrs.insert(sait.key(), sait.value());
                    }
                    result.append(u"<text"_s);
                    for (auto ait = finalAttrs.constBegin(); ait != finalAttrs.constEnd(); ++ait) {
                        result.append(u" %1=\"%2\""_s.arg(ait.key(), ait.value()));
                    }
                    result.append(u">%1</text>"_s.arg(span.body));
                } else {
                    if (!finalAttrs.contains(u"xml:space"_s)) {
                        finalAttrs.insert(u"xml:space"_s, u"preserve"_s);
                    }
                    result.append(u"<text"_s);
                    for (auto ait = finalAttrs.constBegin(); ait != finalAttrs.constEnd(); ++ait) {
                        result.append(u" %1=\"%2\""_s.arg(ait.key(), ait.value()));
                    }
                    result.append(u">"_s);
                    for (const auto &span : line.spans) {
                        if (span.attrs.isEmpty()) {
                            result.append(u"<tspan>%1</tspan>"_s.arg(span.body));
                        } else {
                            result.append(u"<tspan"_s);
                            for (auto sait = span.attrs.constBegin(); sait != span.attrs.constEnd(); ++sait) {
                                result.append(u" %1=\"%2\""_s.arg(sait.key(), sait.value()));
                            }
                            result.append(u">%1</tspan>"_s.arg(span.body));
                        }
                    }
                    result.append(u"</text>"_s);
                }
            }
        }
    }

    result.append(QStringView(cleanSvg).mid(lastPos));
    return result;
}
