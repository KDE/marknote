// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import Qt.labs.platform as Labs

Labs.Menu {
    property Window _window: applicationWindow()
    required property var application

    title: i18nc("@action:menu", "Window")

    NativeMenuItemFromAction {
        action: root.application.action('open_kcommand_bar')
    }

    Labs.MenuItem {
        text: root.visibility === Window.FullScreen ? i18nc("@action:menu", "Exit Full Screen") : i18nc("@action:menu", "Enter Full Screen")
        icon.name: "view-fullscreen"
        shortcut: StandardKey.FullScreen
        onTriggered: if (_window.visibility === Window.FullScreen) {
            _window.showNormal();
        } else {
            _window.showFullScreen();
        }
    }
}
