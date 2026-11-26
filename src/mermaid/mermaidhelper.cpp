// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidhelper.h"
#include "mermaidcssflattener.h"
#include "mermaidimageprovider.h"
#include <marknotesettings.h>
#include <QCryptographicHash>
#include <QFont>
#include <QFontDatabase>
#include <QRegularExpression>
#include <algorithm>

using namespace Qt::StringLiterals;

MermaidHelper::MermaidHelper(QObject *parent)
    : QObject(parent)
{
}

QString MermaidHelper::diagramId(const QString &code, bool isDark)
{
    const QString trimmedCode = code.trimmed();
    if (trimmedCode.isEmpty()) {
        return QString();
    }

    const QString hash = QString::fromLatin1(QCryptographicHash::hash(trimmedCode.toUtf8(), QCryptographicHash::Sha256).toHex().left(16));
    MermaidImageProvider::registerDiagram(hash, trimmedCode);

    const QFont font = Config::editorFont();
    QString fontFamily = font.family().isEmpty() ? QFontDatabase::systemFont(QFontDatabase::GeneralFont).family() : font.family();
    if (fontFamily.isEmpty()) {
        fontFamily = u"sans-serif"_s;
    }
    const qreal fontSize = font.pointSizeF() > 0 ? font.pointSizeF() : (font.pixelSize() > 0 ? font.pixelSize() : 10.0);

    QHash<QString, QString> vars = MermaidCssFlattener::defaultVariables(isDark);
    vars[u"--pele-font"_s] = fontFamily;

    QStringList keys = vars.keys();
    std::sort(keys.begin(), keys.end());
    QCryptographicHash themeHasher(QCryptographicHash::Sha256);
    for (const QString &k : keys) {
        themeHasher.addData(k.toUtf8());
        themeHasher.addData("=");
        themeHasher.addData(vars.value(k).toUtf8());
        themeHasher.addData("\n");
    }
    const QString themeTag = QString::fromLatin1(themeHasher.result().toHex().left(8));

    const QString id = u"%1_%2_%3_%4_%5"_s.arg(hash).arg(isDark ? 1 : 0).arg(themeTag, fontFamily).arg(fontSize);
    MermaidImageProvider::registerRenderContext(id, {fontFamily, fontSize, std::move(vars)});
    return id;
}

QString MermaidHelper::getCacheKey(const QString &code, bool isDark, int widthBucket, qreal dpr)
{
    Q_UNUSED(widthBucket);
    Q_UNUSED(dpr);
    return diagramId(code, isDark);
}

bool MermaidHelper::isMermaid(const QString &language, const QString &code)
{
    const QString lang = language.trimmed().toLower();
    if (lang == u"mermaid"_s || lang == u"mmd"_s) {
        return true;
    }
    if (!lang.isEmpty()) {
        return false;
    }

    const QStringList lines = code.split(QLatin1Char('\n'));
    QString firstMeaningful;
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(u"%%"_s)) {
            continue;
        }
        firstMeaningful = trimmed;
        break;
    }

    if (firstMeaningful.isEmpty()) {
        return false;
    }

    static const QRegularExpression diagRegex(
        u"^(flowchart|graph|sequenceDiagram|classDiagram|stateDiagram(?:-v2)?|erDiagram|gantt|pie|journey|gitGraph|mindmap|timeline|quadrantChart|requirementDiagram|C4Context|C4Container|C4Component|C4Dynamic|C4Deployment|sankey(?:-beta)?|xychart(?:-beta)?|block(?:-beta)?|radar(?:-beta)?|treeView(?:-beta)?|treemap(?:-beta)?|venn(?:-beta)?|wardley(?:-beta)?|swimlane(?:-beta)?|cynefin(?:-beta)?|architecture(?:-beta)?|agentflow(?:-beta)?|railroad(?:-(?:ebnf|abnf|peg))?(?:-beta)?|eventmodeling|kanban|packet(?:-beta)?|usecase(?:-beta)?)\\b"_s,
        QRegularExpression::CaseInsensitiveOption);

    return diagRegex.match(firstMeaningful).hasMatch();
}
