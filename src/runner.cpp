// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QDBusMetaType>

#include "notebooksmodel.h"
#include "runner.h"

/**
 * @brief Constructor - Registers D-Bus meta-types for KRunner RemoteMatch communication
 * @param parent Parent QObject for memory management
 */
Runner::Runner(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<RemoteMatch>();
    qDBusRegisterMetaType<RemoteMatches>();
    qDBusRegisterMetaType<RemoteAction>();
    qDBusRegisterMetaType<RemoteActions>();
}

/**
 * @brief Sets the NoteBooksModel and emits change signal for QML binding
 * @param model New NoteBooksModel pointer (nullptr disconnects)
 */
void Runner::setModel(NoteBooksModel *model)
{
    if (m_model == model) { // Idempotent - no change if same model
        return;
    }
    m_model = model;
    Q_EMIT modelChanged(); // Updates QML bindings
}

/**
 * @brief Empty Teardown slot - satisfies KRunner D-Bus interface requirement
 *
 * Fixes warning "qt.dbus.integration: Could not find slot Runner::Teardown"
 * Leave empty unless cache cleanup needed.
 */
void Runner::Teardown()
{
}

/**
 * @brief Returns empty actions list - no custom actions beyond selection
 * @return Empty RemoteActions list
 */
RemoteActions Runner::Actions()
{
    return {};
}

/**
 * @brief Launches notebook on selection - handles Wayland activation token
 * @param id Notebook path ID from Match() result
 * @param actionId Unused - KRunner default action only
 */
void Runner::Run(const QString &id, const QString &actionId)
{
    Q_UNUSED(actionId);

#ifdef HAVE_KWINDOWSYSTEM
    if (KWindowSystem::isPlatformWayland()) {
        if (!m_activationToken.isEmpty()) {
            KWindowSystem::setCurrentXdgActivationToken(m_activationToken);
        }
    }
#endif

    Q_EMIT notebookSelected(id); // Triggers QML notebook opening
}

/**
 * @brief Searches notebooks model for matches against search term for KRunner
 * @param searchTerm Search query from KRunner (case-insensitive)
 * @return RemoteMatches list with matching notebooks (path ID, name, icon, relevance=1.0, type=100)
 */
RemoteMatches Runner::Match(const QString &searchTerm)
{
    RemoteMatches matches;

    if (!m_model) {
        return matches;
    }

    int count = m_model->rowCount(QModelIndex());

    QString term = searchTerm.toLower();

    for (int i = 0; i < count; ++i) {
        QModelIndex idx = m_model->index(i, 0);
        QString name = m_model->data(idx, NoteBooksModel::Role::Name).toString();

        if (name.toLower().contains(term)) {
            RemoteMatch match;

            match.id = m_model->data(idx, NoteBooksModel::Role::Path).toString();
            match.text = name;
            match.iconName = m_model->data(idx, NoteBooksModel::Role::Icon).toString();
            match.relevance = 1.0;
            match.type = 100;
            matches << match;
        }
    }

    return matches;
}

/**
 * @brief Sets the activation token for the runner to allow window focus stealing
 * @param token The activation token provided by KRunner/Wayland
 */
void Runner::SetActivationToken(const QString &token)
{
    m_activationToken = token;
}

QDBusArgument &operator<<(QDBusArgument &argument, const RemoteMatch &match)
{
    argument.beginStructure();
    argument << match.id << match.text << match.iconName << match.type << match.relevance << match.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteMatch &match)
{
    argument.beginStructure();
    argument >> match.id >> match.text >> match.iconName >> match.type >> match.relevance >> match.properties;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const RemoteAction &action)
{
    argument.beginStructure();
    argument << action.id << action.text << action.iconName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteAction &action)
{
    argument.beginStructure();
    argument >> action.id >> action.text >> action.iconName;
    argument.endStructure();
    return argument;
}

#include "moc_runner.cpp"
