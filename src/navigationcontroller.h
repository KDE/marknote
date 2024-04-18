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

    /// This property holds the current note name.
    Q_PROPERTY(QString noteName READ noteName NOTIFY notePathChanged)

    /// This property holds the current note path.
    Q_PROPERTY(QUrl noteFullPath READ noteFullPath NOTIFY notePathChanged)

    /// This property holds whether we are in mobile mode.
    Q_PROPERTY(bool mobileMode READ mobileMode WRITE setMobileMode NOTIFY mobileModeChanged)

public:
    explicit NavigationController(QObject *parent = nullptr);

    QString notebookPath() const;
    void setNotebookPath(const QString &notebookPath);

    QString notebookName() const;

    QString notePath() const;
    void setNotePath(const QString &notePath);

    QString noteName() const;
    QUrl noteFullPath() const;

    bool mobileMode() const;
    void setMobileMode(bool mobileMode);

Q_SIGNALS:
    void notebookPathChanged();
    void notebookIconNameChanged();
    void notebookColorChanged();
    void notePathChanged();
    void mobileModeChanged();

private:
    QString m_notebookPath;
    QString m_notePath;
    bool m_mobileMode;
};
