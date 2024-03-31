// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#include "navigationcontroller.h"

NavigationController::NavigationController(QObject *parent)
    : QObject(parent)
{
}

QString NavigationController::notebookPath() const
{
    return m_notebookPath;
}

QString NavigationController::notebookName() const
{
    return m_notebookPath.split(QLatin1Char('/')).constLast();
}

void NavigationController::setNotebookPath(const QString &notebookPath)
{
    if (m_notebookPath == notebookPath) {
        return;
    }
    m_notebookPath = notebookPath;
    Q_EMIT notebookPathChanged();
}

QString NavigationController::notePath() const
{
    return m_notePath;
}

void NavigationController::setNotePath(const QString &notePath)
{
    if (m_notePath == notePath) {
        return;
    }
    m_notePath = notePath;
    Q_EMIT notePathChanged();
}
