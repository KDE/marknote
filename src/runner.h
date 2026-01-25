// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDBusArgument>
#include <QDBusContext>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

#ifdef HAVE_KWINDOWSYSTEM
#include <KWindowSystem>
#endif

enum MatchType {
    NoMatch = 0,
    CompletionMatch = 10,
    PossibleMatch = 30,
    InformationalMatch = 50,
    HelperMatch = 70,
    ExactMatch = 100,
};

struct RemoteMatch {
    QString id;
    QString text;
    QString iconName;
    int type = MatchType::NoMatch;
    double relevance = 0;
    QVariantMap properties;
};
typedef QList<RemoteMatch> RemoteMatches;

struct RemoteAction {
    QString id;
    QString text;
    QString iconName;
};
typedef QList<RemoteAction> RemoteActions;

// Helper to register types with D-Bus
Q_DECLARE_METATYPE(RemoteMatch)
Q_DECLARE_METATYPE(RemoteMatches)
Q_DECLARE_METATYPE(RemoteAction)
Q_DECLARE_METATYPE(RemoteActions)

class NoteBooksModel;

class Runner : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner.marknote")
    Q_PROPERTY(NoteBooksModel *model READ model WRITE setModel NOTIFY modelChanged)

public:
    explicit Runner(QObject *parent = nullptr);

    void setModel(NoteBooksModel *model);
    NoteBooksModel *model() const
    {
        return m_model;
    }

public Q_SLOTS:
    Q_SCRIPTABLE RemoteActions Actions();
    Q_SCRIPTABLE RemoteMatches Match(const QString &searchTerm);
    Q_SCRIPTABLE void Run(const QString &id, const QString &actionId);
    Q_SCRIPTABLE void SetActivationToken(const QString &token);
    Q_SCRIPTABLE void Teardown();

Q_SIGNALS:
    void modelChanged();
    void notebookSelected(const QString &path);

private:
    NoteBooksModel *m_model = nullptr;
    QString m_activationToken;
};

// D-Bus Argument operators (needed for the DBus service to understand the structs)
QDBusArgument &operator<<(QDBusArgument &argument, const RemoteMatch &match);
const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteMatch &match);
QDBusArgument &operator<<(QDBusArgument &argument, const RemoteAction &action);
const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteAction &action);
