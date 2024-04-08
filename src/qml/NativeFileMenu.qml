// SPDX-FileCopyrightText: 2022 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import Qt.labs.platform as Labs
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.marknote

Labs.Menu {
    id: fileMenu
    title: i18nc("@action:menu", "File")

    default property list<QtObject> additionalMenuItems

    property list<QtObject> _menuItems: [
        StatefulAppLabs.NativeMenuItem {
            application: App
            actionName: 'file_quit'
        }
    ]

    Component.onCompleted: {
        for (let i in additionalMenuItems) {
            fileMenu.addItem(additionalMenuItems[i])
        }
        for (let j in _menuItems) {
            fileMenu.addItem(_menuItems[j])
        }
    }
}
