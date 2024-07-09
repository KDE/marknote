// SPDX-FileCopyrightText: 2023 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "application.h"

#include <KAboutData>
#include <KAuthorized>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDebug>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

App::App(QObject *parent)
    : AbstractKirigamiApplication(parent)
{
    setupActions();
}

QString App::iconName(const QIcon &icon)
{
    return icon.name();
}

void App::setupActions()
{
    AbstractKirigamiApplication::setupActions();

    auto actionName = "add_notebook"_L1;
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mainCollection()->addAction(actionName, this, &App::newNotebook);
        action->setText(i18nc("@action:inmenu", "New Notebook"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("list-add-symbolic")));
        mainCollection()->addAction(action->objectName(), action);
        mainCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    }

    actionName = "add_note"_L1;
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mainCollection()->addAction(actionName, this, &App::newNote);
        action->setText(i18nc("@action:inmenu", "New Note"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("list-add-symbolic")));
        mainCollection()->addAction(action->objectName(), action);
        mainCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_N));
    }

    actionName = "options_configure"_L1;
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = KStandardActions::preferences(this, &App::preferences, this);
        mainCollection()->addAction(action->objectName(), action);
    }

    actionName = QLatin1String("import_knotes");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mainCollection()->addAction(actionName, this, &App::importFromKNotes);
        action->setIcon(QIcon::fromTheme(QStringLiteral("knotes")));
        action->setText(i18nc("@action:inmenu", "Import from KNotes"));
    }

    actionName = QLatin1String("import_maildir");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mainCollection()->addAction(actionName, this, &App::importFromMaildir);
        action->setIcon(QIcon::fromTheme(QStringLiteral("folder-mail")));
        action->setText(i18nc("@action:inmenu", "Import from Maildir"));
    }

    readSettings();
}

#include "moc_application.cpp"
