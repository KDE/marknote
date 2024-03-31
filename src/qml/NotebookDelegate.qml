// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

import QtCore
import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.marknote
import org.kde.kirigamiaddons.delegates as Delegates

Delegates.RoundedItemDelegate {
    id: root

    required property int index
    required property string name
    required property string path
    required property string iconName
    required property string color
    required property var model

    icon.name: iconName
    text: name
    highlighted: NavigationController.notebookPath === path

    contentItem: ColumnLayout {
        Kirigami.Icon {
            source: root.icon.name
            Layout.alignment: Qt.AlignHCenter
        }

        Controls.Label {
            text: root.name
            horizontalAlignment: Qt.AlignHCenter
            elide: Text.ElideRight

            Layout.fillWidth: true
        }
    }

    TapHandler {
        onTapped: NavigationController.notebookPath = path
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            const menuComponent = Qt.createComponent("org.kde.marknote", "NotebookContextMenu");
            const menu = menuComponent.createObject(root, {
                path: root.path,
                name: root.name,
                model: root.model
            });
            menu.popup();
        }
    }

    Layout.fillWidth: true

    Controls.ToolTip.text: text
    Controls.ToolTip.visible: hovered
    Controls.ToolTip.delay: Kirigami.Units.toolTipDelay
}
