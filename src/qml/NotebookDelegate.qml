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
    activeFocusOnTab: true

    Behavior on implicitHeight {
        NumberAnimation {
            id: mainAnim
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.InOutQuart
        }
    }


    function updateColor(): void {
        if (color !== '#ffffff' && color !== '#00000000') {
            root.background.Kirigami.Theme.highlightColor = color;
        } else if (root.background.Kirigami.Theme.highlightColor !== applicationWindow().Kirigami.Theme.highlightColor) {
            root.background.Kirigami.Theme.highlightColor = applicationWindow().Kirigami.Theme.highlightColor;
        }
    }

    onColorChanged: updateColor();
    Component.onCompleted: updateColor();

    contentItem: Item{
        implicitHeight: Config.expandedSidebar ? icon.height + 2 * Kirigami.Units.smallSpacing : icon.height + label.height + 3 * Kirigami.Units.smallSpacing
        Kirigami.Icon {
            id: icon
            source: root.icon.name
            y: Kirigami.Units.smallSpacing
            x: Config.expandedSidebar ? parent.x:  (80 - Kirigami.Units.largeSpacing * 2) / 2 - width / 2
            Behavior on x {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }
        }
        Controls.Label {
            id: label
            text: root.name
            width: Math.min(parent.width, implicitWidth)
            elide: Text.ElideRight
            y: Config.expandedSidebar ? parent.implicitHeight / 2 - height / 2 : icon.height + icon. y + Kirigami.Units.smallSpacing
            x: Config.expandedSidebar ? icon.width + Kirigami.Units.largeSpacing * 2 : (80 - Kirigami.Units.largeSpacing * 2) / 2 - Math.min((80 - Kirigami.Units.largeSpacing * 2), implicitWidth) / 2
            Behavior on x {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }
            Behavior on y {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }
        }
        Controls.ToolButton {
            x: parent.width - width - Kirigami.Units.smallSpacing
            y: parent.height / 2 - height / 2
            icon.name: "overflow-menu"
            onClicked: {
                const menuComponent = Qt.createComponent("org.kde.marknote", "NotebookContextMenu");
                const menu = menuComponent.createObject(root, {
                    path: root.path,
                    name: root.name,
                    model: root.model
                });
                menu.popup();
            }
            opacity: Config.expandedSidebar ? 1 : 0
            enabled: Config.expandedSidebar
            Behavior on opacity {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }
        }
    }
//    contentItem: GridLayout {
//        flow: Config.expandedSidebar ? GridLayout.LeftToRight : GridLayout.TopToBottom

//        Kirigami.Icon {
//            source: root.icon.name
//            Layout.alignment: Qt.AlignHCenter

//        }

//        Controls.Label {
//            text: root.name
//            horizontalAlignment: Config.expandedSidebar ? Qt.AlignLeft : Qt.AlignHCenter
//            elide: Text.ElideRight

//            Layout.fillWidth: true
//        }
//    }

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
