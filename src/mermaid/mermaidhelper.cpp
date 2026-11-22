// SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mermaidhelper.h"
#include "mermaidimageprovider.h"

#include <QCryptographicHash>
#include <QRegularExpression>

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

    return u"%1_%2"_s.arg(hash).arg(isDark ? 1 : 0);
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
        u"^(flowchart|graph|sequenceDiagram|classDiagram|stateDiagram(?:-v2)?|erDiagram|gantt|pie|journey|gitGraph|mindmap|timeline|quadrantChart|requirementDiagram|C4Context|C4Container|C4Component|sankey-beta|xychart-beta|block-beta)\\b"_s,
        QRegularExpression::CaseInsensitiveOption);

    return diagRegex.match(firstMeaningful).hasMatch();
}
