// SPDX-FileCopyrightText: 2023 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "application.h"

#include "commandbarfiltermodel.h"
#include <KAboutData>
#include <KAuthorized>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <QDebug>
#include <QGuiApplication>
#include <QMenu>

App::App(QObject *parent)
    : QObject(parent)
    , mCollection(new KActionCollection(parent))
{
    setupActions();
}

App::~App()
{
    if (m_actionModel) {
        auto lastUsedActions = m_actionModel->lastUsedActions();
        auto cfg = KSharedConfig::openConfig();
        KConfigGroup cg(cfg, QStringLiteral("General"));
        cg.writeEntry("CommandBarLastUsedActions", lastUsedActions);
    }
}

QList<KActionCollection *> App::actionCollections() const
{
    return {mCollection};
}

/**
 * A helper function that takes a list of KActionCollection* and converts it
 * to KCommandBar::ActionGroup
 */
static QList<KalCommandBarModel::ActionGroup> actionCollectionToActionGroup(const QList<KActionCollection *> &actionCollections)
{
    using ActionGroup = KalCommandBarModel::ActionGroup;

    QList<ActionGroup> actionList;
    actionList.reserve(actionCollections.size());

    for (const auto collection : actionCollections) {
        const auto collectionActions = collection->actions();
        const auto componentName = collection->componentDisplayName();

        ActionGroup ag;
        ag.name = componentName;
        ag.actions.reserve(collection->count());
        for (const auto action : collectionActions) {
            /**
             * If this action is a menu, fetch all its child actions
             * and skip the menu action itself
             */
            if (const auto menu = action->menu()) {
                const auto menuActions = menu->actions();

                ActionGroup menuActionGroup;
                menuActionGroup.name = KLocalizedString::removeAcceleratorMarker(action->text());
                menuActionGroup.actions.reserve(menuActions.size());
                for (const auto mAct : menuActions) {
                    if (mAct) {
                        menuActionGroup.actions.append(mAct);
                    }
                }

                /**
                 * If there were no actions in the menu, we
                 * add the menu to the list instead because it could
                 * be that the actions are created on demand i.e., aboutToShow()
                 */
                if (!menuActions.isEmpty()) {
                    actionList.append(menuActionGroup);
                    continue;
                }
            }

            if (action && !action->text().isEmpty()) {
                ag.actions.append(action);
            }
        }
        actionList.append(ag);
    }
    return actionList;
}

QSortFilterProxyModel *App::actionsModel()
{
    if (!m_proxyModel) {
        m_actionModel = new KalCommandBarModel(this);
        m_proxyModel = new CommandBarFilterModel(this);
        m_proxyModel->setSortRole(KalCommandBarModel::Score);
        m_proxyModel->setFilterRole(Qt::DisplayRole);
        m_proxyModel->setSourceModel(m_actionModel);
    }

    // setLastUsedActions
    auto cfg = KSharedConfig::openConfig();
    KConfigGroup cg(cfg, QStringLiteral("General"));

    const auto actionNames = cg.readEntry(QStringLiteral("CommandBarLastUsedActions"), QStringList());

    m_actionModel->setLastUsedActions(actionNames);
    m_actionModel->refresh(actionCollectionToActionGroup(actionCollections()));
    return m_proxyModel;
}

void App::configureShortcuts()
{
    // TODO replace with QML version
    KShortcutsDialog dlg(KShortcutsEditor::ApplicationAction, KShortcutsEditor::LetterShortcutsAllowed, nullptr);
    dlg.setModal(true);
    const auto collections = actionCollections();
    for (const auto collection : collections) {
        dlg.addCollection(collection);
    }
    dlg.configure();
}

QAction *App::action(const QString &name)
{
    const auto collections = actionCollections();
    for (const auto collection : collections) {
        auto resultAction = collection->action(name);
        if (resultAction) {
            return resultAction;
        }
    }

    qWarning() << "Not found action for name" << name;

    return nullptr;
}

void App::setupActions()
{
    auto actionName = QLatin1String("open_kcommand_bar");
    if (KAuthorized::authorizeAction(actionName)) {
        auto openKCommandBarAction = mCollection->addAction(actionName, this, &App::openKCommandBarAction);
        openKCommandBarAction->setText(i18n("Open Command Bar"));
        openKCommandBarAction->setIcon(QIcon::fromTheme(QStringLiteral("new-command-alarm")));

        mCollection->addAction(openKCommandBarAction->objectName(), openKCommandBarAction);
        mCollection->setDefaultShortcut(openKCommandBarAction, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
    }

    actionName = QLatin1String("file_quit");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = KStandardAction::quit(this, &App::quit, this);
        mCollection->addAction(action->objectName(), action);
    }

    actionName = QLatin1String("options_configure_keybinding");
    if (KAuthorized::authorizeAction(actionName)) {
        auto keyBindingsAction = KStandardAction::keyBindings(this, &App::configureShortcuts, this);
        mCollection->addAction(keyBindingsAction->objectName(), keyBindingsAction);
    }

    actionName = QLatin1String("open_about_page");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mCollection->addAction(actionName, this, &App::openAboutPage);
        action->setText(i18n("About %1", KAboutData::applicationData().displayName()));
        action->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    }

    actionName = QLatin1String("open_about_kde_page");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mCollection->addAction(actionName, this, &App::openAboutKDEPage);
        action->setText(i18n("About KDE"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    }

    // actionName = QLatin1String("options_configure");
    // if (KAuthorized::authorizeAction(actionName)) {
    //     auto action = KStandardAction::preferences(this, &App::openSettings, this);
    //     mCollection->addAction(action->objectName(), action);
    // }

    actionName = QLatin1String("add_notebook");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mCollection->addAction(actionName, this, &App::newNotebook);
        action->setText(i18nc("@action:inmenu", "New Notebook"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("list-add-symbolic")));
        mCollection->addAction(action->objectName(), action);
        mCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    }

    actionName = QLatin1String("add_note");
    if (KAuthorized::authorizeAction(actionName)) {
        auto action = mCollection->addAction(actionName, this, &App::newNote);
        action->setText(i18nc("@action:inmenu", "New Note"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("list-add-symbolic")));
        mCollection->addAction(action->objectName(), action);
        mCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_N));
    }
}

void App::quit()
{
    qGuiApp->exit();
}

QString App::iconName(const QIcon &icon) const
{
    return icon.name();
}

#include "moc_application.cpp"
