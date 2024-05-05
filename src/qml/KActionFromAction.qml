// SPDX-FileCopyrightText: 2021 Claudio Cambra <claudio.cambra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.marknote

Kirigami.Action {
    required property var action

    text: action.text
    shortcut: action.shortcut
    icon.name: App.iconName(action.icon)
    onTriggered: action.trigger()
    visible: action.text.length > 0
    checkable: action.checkable
    checked: action.checked
    enabled: action.enabled
}
