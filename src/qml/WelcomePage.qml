// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.ki18n

Kirigami.Page {
    id: root

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        icon.name: "addressbook-details"
        text: KI18n.i18n("Start by creating your first notebook!")
        helpfulAction: newNotebookAction
    }

    actions: Kirigami.Action {
        visible: ApplicationWindow.window.visibility === Window.FullScreen
        icon.name: "window-restore-symbolic"
        tooltip: KI18n.i18nc("@action:menu", "Exit Full Screen")
        checkable: true
        checked: true
        onTriggered: ApplicationWindow.window.showNormal()
    }
}
