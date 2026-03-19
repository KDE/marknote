// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import Qt.labs.platform as Labs

import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.marknote
import org.kde.ki18n

Labs.Menu {
    id: root

    // Leave this empty so the parent can pass the window in
    property Window _window

    title: KI18n.i18nc("@action:menu", "Window")

    StatefulAppLabs.NativeMenuItem {
        actionName: 'open_kcommand_bar'
        application: App // qmllint disable incompatible-type
    }

    Labs.MenuItem {
        text: (root._window && root._window.visibility === Window.FullScreen)
        ? KI18n.i18nc("@action:menu", "Exit Full Screen")
        : KI18n.i18nc("@action:menu", "Enter Full Screen")
        icon.name: "view-fullscreen"
        shortcut: StandardKey.FullScreen

        onTriggered: {
            if (root._window) {
                if (root._window.visibility === Window.FullScreen) {
                    root._window.showNormal();
                } else {
                    root._window.showFullScreen();
                }
            }
        }
    }
}
