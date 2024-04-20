// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2015-2024 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: BSD-3-Clause AND LGPL-2.0-or-later

#include "latextopng.h"

#include <fstream>
#include <iostream>

#include <KLocalizedString>
#include <KStandardShortcut>

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlFile>

using namespace Qt::StringLiterals;

LatexToPNG::LatexToPNG(QObject *parent)
    : QObject(parent)
{
}

void LatexToPNG::renderPNG(QString latexText)
{
    QStringList latexArguments = {QString::fromUtf8("-halt-on-error"), QString::fromUtf8("tempfile.tex")};

    QString latexfileContent = QString::fromUtf8("\\documentclass{standalone}\n\\usepackage{amsmath}\n\\begin{document}\n%1\n\\end{document}").arg(latexText);

    QFile outfile(u"tempfile.tex"_s);
    if (!outfile.open(QIODeviceBase::WriteOnly)) {
        return;
    }
    outfile.write(latexfileContent.toUtf8());
    outfile.close();

    QProcess *latexProcess = new QProcess();

    connect(latexProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus != QProcess::NormalExit) {
            return;
        }

        QProcess *dvipngProcess = new QProcess();
        QStringList dvipngArguments = {QString::fromUtf8("tempfile.dvi")};

        dvipngProcess->start(QString::fromUtf8("dvipng"), dvipngArguments);
        connect(dvipngProcess, &QProcess::finished, this, [](int exitCode, QProcess::ExitStatus exitStatus) {
            qWarning() << "done" << exitStatus;
        });

        connect(dvipngProcess, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
            qWarning() << "error" << error;
        });

        qWarning() << "rejoire";
    });

    latexProcess->start(QString::fromUtf8("latex"), latexArguments);
}
