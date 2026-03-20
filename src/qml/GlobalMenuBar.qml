// SPDX-FileCopyrightText: 2024 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import Qt.labs.platform as Labs
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.marknote
import org.kde.ki18n

Labs.MenuBar {
    id: root

    property var _window: ApplicationWindow.window

    NativeFileMenu {}

    NativeEditMenu {}

    NativeWindowMenu {
        _window: root._window
    }

    Labs.Menu {
        title: KI18n.i18nc("@action:menu", "Settings")

        StatefulAppLabs.NativeMenuItem {
            actionName: 'options_configure_keybinding'
            application: App
        }
        StatefulAppLabs.NativeMenuItem {
            actionName: 'options_configure'
            application: App
        }
    }

    NativeHelpMenu {}
}
