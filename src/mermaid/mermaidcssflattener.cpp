// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidcssflattener.h"
#include <marknotesettings.h>

#include <KColorScheme>

#include <QColor>
#include <QFontDatabase>
#include <QGuiApplication>
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

        auto toHex = [](const QColor &c) -> QString { return c.name(QColor::HexRgb); };

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
    aliases[u"--_hw"_s] = vars.value(u"--pele-heading-weight"_s, u"bold"_s);
    aliases[u"--_tw"_s] = vars.value(u"--pele-title-weight"_s, u"bold"_s);

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

    return svg;
}
