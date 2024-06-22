/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

#include <KAboutData>
#include <KCrash>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QUrl>

#include "../marknote-version.h"
#include "colorschemer.h"
#include "config.h"
#include "windowcontroller.h"

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

#ifdef WITH_BREEZEICONS_LIB
#include <BreezeIcons>
#endif

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QIcon::setFallbackThemeName(u"breeze"_s);
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    KCrash::initialize();

#ifdef WITH_BREEZEICONS_LIB
    BreezeIcons::initIcons();
#endif

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

#ifdef Q_OS_MACOS
    QApplication::setStyle(QStringLiteral("breeze"));
#endif

    KLocalizedString::setApplicationDomain("marknote");

    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.marknote")));

    KAboutData about(QStringLiteral("marknote"),
                     i18nc("Application name", "Marknote"),
                     QStringLiteral(MARKNOTE_VERSION_STRING),
                     i18n("Note taking application"),
                     KAboutLicense::GPL_V2,
                     i18n("© 2023 Mathis Brüchert"));
    about.addAuthor(i18n("Mathis Brüchert"),
                    i18n("Maintainer"),
                    QStringLiteral("mbb@kaidan.im"),
                    QStringLiteral("https://invent.kde.org/mbruchert"),
                    QUrl(QStringLiteral("https://gravatar.com/avatar/f9c35f242fe79337bf8746ca9fccc189?size=256.png")));
    about.addAuthor(i18n("Carl Schwan"),
                    i18n("Maintainer"),
                    QStringLiteral("carl@carlschwan.eu"),
                    QStringLiteral("https://carlschwan.eu"),
                    QUrl(QStringLiteral("https://carlschwan.eu/avatar.png")));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    KAboutData::setApplicationData(about);

    ColorSchemer colorScheme;
    if (!MarknoteSettings::self()->colorScheme().isEmpty()) {
        colorScheme.apply(MarknoteSettings::self()->colorScheme());
    }

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    if (parser.positionalArguments().length() > 0) {
        const auto path = parser.positionalArguments()[0];
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

    QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit, QApplication::instance(), [] {
        MarknoteSettings::self()->save();
    });

    QQuickWindow *window = nullptr;

    const auto rootObjects = engine.rootObjects();
    for (auto obj : rootObjects) {
        auto view = qobject_cast<QQuickWindow *>(obj);
        if (view) {
            window = view;
            break;
        }
    }

    if (window != nullptr) {
        auto controller = engine.singletonInstance<WindowController *>(QStringLiteral("org.kde.marknote"), QStringLiteral("WindowController"));
        controller->setWindow(window);
        controller->restoreGeometry();
    }

    return app.exec();
}
