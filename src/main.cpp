/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

#include <KLocalizedContext>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QtQml>

#include "documenthandler.h"
#include "notesmodel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("marknote");

    QQmlApplicationEngine engine;

    qmlRegisterType<DocumentHandler>("org.kde.marknote", 1, 0, "DocumentHandler");
    qmlRegisterType<NotesModel>("org.kde.marknote", 1, 0, "NotesModel");
    qmlRegisterAnonymousType<QAbstractItemModel>("org.kde.marknote", 1);
    qmlRegisterType<QSortFilterProxyModel>("org.kde.marknote", 1, 0, "SortFilterModel");

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
