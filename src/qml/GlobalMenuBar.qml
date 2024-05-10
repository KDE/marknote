// SPDX-FileCopyrightText: 2024 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform as Labs

Labs.MenuBar {
    id: root

    required property var application

    NativeFileMenu {}

    NativeEditMenu {}

    NativeWindowMenu {
        application: root.application
    }

    Labs.Menu {
        title: i18nc("@action:menu", "Settings")

        NativeMenuItemFromAction {
            action: root.application.action('options_configure_keybinding')
        }
        NativeMenuItemFromAction {
            action: root.application.action('options_configure')
        }
    }

    NativeHelpMenu {
        application: root.application
    }
}
