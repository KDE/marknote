/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QtQml>

#include "../marknote-version.h"
#include "documenthandler.h"
#include "notebooksmodel.h"
#include "notesmodel.h"

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    auto font = app.font();
    font.setPointSize(10);
    app.setFont(font);
#endif
    KLocalizedString::setApplicationDomain("marknote");

    QGuiApplication::setOrganizationName("KDE");
    QGuiApplication::setApplicationName("marknote");
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.marknote")));

    KAboutData about(QStringLiteral("marknote"),
                     i18n("Marknote"),
                     QStringLiteral(MARKNOTE_VERSION_STRING),
                     i18n("Note taking application"),
                     KAboutLicense::GPL_V2,
                     i18n("© 2023 Mathis Brüchert"));
    about.addAuthor(i18n("Mathis Brüchert"), i18n("Maintainer"), QStringLiteral("mbb-mail@gmx.de"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");
    about.setBugAddress("https://bugs.kde.org/describecomponents.cgi?product=marknote");

    QQmlApplicationEngine engine;

    qmlRegisterType<DocumentHandler>("org.kde.marknote", 1, 0, "DocumentHandler");
    qmlRegisterType<NotesModel>("org.kde.marknote", 1, 0, "NotesModel");
    qmlRegisterType<NoteBooksModel>("org.kde.marknote", 1, 0, "NoteBooksModel");

    qmlRegisterAnonymousType<QAbstractItemModel>("org.kde.marknote", 1);
    qmlRegisterType<QSortFilterProxyModel>("org.kde.marknote", 1, 0, "SortFilterModel");

    qmlRegisterSingletonType("org.kde.marknote", 1, 0, "About", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
