// SPDX-FileCopyrightText: 2020 (c) Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import Qt.labs.platform
import org.kde.marknote

MenuItem {
    required property var action

    text: action.text
    shortcut: action.shortcut
    icon.name: App.iconName(action.icon)
    onTriggered: action.trigger()
    visible: action.text.length > 0
    checkable: action.checkable
    checked: action.checked
    enabled: action.enabled && parent.enabled
}
