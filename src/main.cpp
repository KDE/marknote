/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
    SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
*/

#include <KAboutData>
#include <KirigamiAppDefaults>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 19, 0) && __has_include("KCrash")
#include <KCrash>
#endif
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>
#ifndef Q_OS_ANDROID
#include <QApplication>
#endif
#include <QCommandLineParser>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtSystemDetection>
#if !defined(Q_OS_ANDROID)
#include <QDBusConnection>
#include <QDBusError>
#endif

#include "../marknote-version.h"
#include "colorschemer.h"
#include "sketchhistory.h"
#include <QUrl>
#include <marknotesettings.h>
#ifdef HAVE_KRUNNER
#include "runner.h"
#endif

#if KI18N_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <KLocalizedQmlContext>
#endif

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

#ifdef WITH_BREEZEICONS_LIB
#include <BreezeIcons>
#endif

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    KirigamiAppDefaults::apply(&app);

#ifdef WITH_BREEZEICONS_LIB
    BreezeIcons::initIcons();
#endif

    KLocalizedString::setApplicationDomain("marknote");
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"org.kde.marknote"_s));

    KAboutData about(u"marknote"_s,
                     i18nc("Application name", "Marknote"),
                     QStringLiteral(MARKNOTE_VERSION_STRING),
                     i18n("Note taking application"),
                     KAboutLicense::GPL_V2,
                     i18n("© 2023-2026 Mathis Brüchert"));
    about.addAuthor(i18n("Mathis Brüchert"),
                    i18n("Maintainer"),
                    u"mbb@kaidan.im"_s,
                    u"https://invent.kde.org/mbruchert"_s,
                    QUrl(u"https://gravatar.com/avatar/f9c35f242fe79337bf8746ca9fccc189?size=256.png"_s));
    about.addAuthor(i18n("Carl Schwan"), i18n("Maintainer"), u"carl@carlschwan.eu"_s, u"https://carlschwan.eu"_s, QUrl(u"https://carlschwan.eu/avatar.png"_s));
    about.addAuthor(i18n("Valentyn Bondarenko"),
                    i18n("Maintainer"),
                    u"bondarenko@vivaldi.net"_s,
                    u"https://invent.kde.org/hunterx"_s,
                    QUrl(u"https://invent.kde.org/uploads/-/system/user/avatar/14675/avatar.png?width=256"_s));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    KAboutData::setApplicationData(about);

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 19, 0) && __has_include("KCrash")
    KCrash::initialize();
#endif

    ColorSchemer colorScheme;
    if (!Config::self()->colorScheme().isEmpty()) {
        colorScheme.apply(Config::self()->colorScheme());
    }

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);

    qRegisterMetaType<Stroke>("Stroke");

    engine.rootContext()->setContextProperty(u"appFontList"_s, QFontDatabase::families());

#ifdef HAVE_KRUNNER
    qmlRegisterType<Runner>("org.kde.marknote", 1, 0, "Runner");
    Runner *runner = new Runner(&app);
    qmlRegisterSingletonInstance<Runner>("org.kde.marknote", 1, 0, "KRunner", runner);

#if !defined(Q_OS_ANDROID)
    if (QDBusConnection::sessionBus().registerService(u"org.kde.marknote"_s)) {
        QDBusConnection::sessionBus().registerObject(u"/NotebookRunner"_s, u"org.kde.krunner1"_s, runner, QDBusConnection::ExportAllContents);
    }
#endif
#endif

    if (parser.positionalArguments().length() > 0) {
        QUrl url(parser.positionalArguments()[0]);
        const auto path = url.isLocalFile() ? url.toLocalFile() : parser.positionalArguments()[0];

        if (QFile::exists(path)) {
            engine.rootContext()->setContextProperty(u"cliNoteName"_s, path.split(QLatin1Char('/')).last().replace(u".md"_s, QString{}));
            engine.rootContext()->setContextProperty(u"cliNoteFullPath"_s, QUrl::fromLocalFile(path));
        }
        engine.loadFromModule(u"org.kde.marknote"_s, u"MainEditor"_s);
    } else {
        engine.loadFromModule(u"org.kde.marknote"_s, u"Main"_s);
    }

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
