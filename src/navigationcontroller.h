// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

#pragma once

#include <QObject>
#include <QQmlEngine>

class NavigationController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /// This property holds the current notebook path.
    Q_PROPERTY(QString notebookPath READ notebookPath WRITE setNotebookPath NOTIFY notebookPathChanged)

    /// This property holds the current notebook name.
    Q_PROPERTY(QString notebookName READ notebookName NOTIFY notebookPathChanged)

    /// This property holds the current note path relative to the notebookPath.
    Q_PROPERTY(QString notePath READ notePath WRITE setNotePath NOTIFY notePathChanged)

public:
    explicit NavigationController(QObject *parent = nullptr);

    QString notebookPath() const;
    void setNotebookPath(const QString &notebookPath);

    QString notebookName() const;

    QString notePath() const;
    void setNotePath(const QString &notePath);

Q_SIGNALS:
    void notebookPathChanged();
    void notebookIconNameChanged();
    void notebookColorChanged();
    void notePathChanged();

private:
    QString m_notebookPath;
    QString m_notePath;
};
