// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#pragma once

#include <AbstractKirigamiApplication>
#include <QObject>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

class App : public AbstractKirigamiApplication
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit App(QObject *parent = nullptr);
    ~App() override = default;

    Q_INVOKABLE [[nodiscard]] static QString iconName(const QIcon &icon);

Q_SIGNALS:
    void newNotebook();
    void newNote();
    void preferences();
    void importFromMaildir();
    void importFromKNotes();

private:
    void setupActions() override;
};
