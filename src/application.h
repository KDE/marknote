// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#pragma once

#include "actionsmodel.h"
#include <KActionCollection>
#include <QObject>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

class App : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QSortFilterProxyModel *actionsModel READ actionsModel CONSTANT)

public:
    explicit App(QObject *parent = nullptr);
    ~App();

    Q_INVOKABLE void configureShortcuts();
    Q_INVOKABLE QAction *action(const QString &actionName);
    Q_INVOKABLE QString iconName(const QIcon &icon) const;

    QList<KActionCollection *> actionCollections() const;
    QSortFilterProxyModel *actionsModel();

Q_SIGNALS:
    void openSettings();
    void openAboutPage();
    void openAboutKDEPage();
    void openKCommandBarAction();
    void newNotebook();

protected:
    virtual void setupActions();
    KActionCollection *mCollection = nullptr;

private:
    void quit();
    KalCommandBarModel *m_actionModel = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
};