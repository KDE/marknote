// SPDX-FileCopyrightText: 2024 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform as Labs
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.marknote

Labs.MenuBar {
    id: root

    NativeFileMenu {}

    NativeEditMenu {}

    NativeWindowMenu {}

    Labs.Menu {
        title: i18nc("@action:menu", "Settings")

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
