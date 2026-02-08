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
    required property int noteCount
    required property var model

    icon.name: iconName
    text: name
    highlighted: NavigationController.notebookPath === path
    activeFocusOnTab: true

    Behavior on implicitHeight {
        NumberAnimation {
            id: mainAnim
            duration: Kirigami.Units.shortDuration * 2
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
            width: Config.expandedSidebar
            ? parent.width - x - (menuButton.width + countLabel.implicitWidth + Kirigami.Units.largeSpacing * 2)
            : Math.min(parent.width, implicitWidth)
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
            id: menuButton
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

        Controls.Label {
            id: countLabel

            visible: Config.expandedSidebar
            opacity: visible ? true : false

            text: root.noteCount
            font.pixelSize: Kirigami.Units.gridUnit / 1.5
            color: root.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: menuButton.left
            anchors.rightMargin: Kirigami.Units.smallSpacing

            Behavior on opacity {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }
        }

        Rectangle {
            id: badge

            visible: !Config.expandedSidebar && root.noteCount > 0
            opacity: visible ? true : false

            readonly property int badgeHeight: Math.round(Kirigami.Units.gridUnit * 0.8)
            readonly property int badgePadding: Kirigami.Units.smallSpacing
            readonly property int badgeMaxWidth: icon.width

            height: badgeHeight
            width: Math.min(badgeMaxWidth, Math.max(badgeHeight, textBadge.implicitWidth + badgePadding * 2))
            radius: height / 2

            anchors.verticalCenter: icon.top
            anchors.horizontalCenter: icon.right

            anchors.verticalCenterOffset: Kirigami.Units.smallSpacing / 2
            anchors.horizontalCenterOffset: -Kirigami.Units.smallSpacing / 2

            color: root.highlighted
                   ? root.background.Kirigami.Theme.highlightColor
                     : Kirigami.Theme.alternateBackgroundColor
            Behavior on opacity {
                NumberAnimation {
                    duration: mainAnim.duration
                    easing.type: mainAnim.easing.type
                }
            }

            Text {
                id: textBadge
                text: root.noteCount
                anchors.centerIn: parent
                color: Kirigami.Theme.textColor
                font.pixelSize: Math.round(Kirigami.Units.gridUnit * 0.6)
                width: parent.width - badge.badgePadding * 2
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
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

    Controls.ToolTip {
        id: myToolTip
        text: root.text
        visible: root.hovered && label.truncated
        delay: Kirigami.Units.toolTipDelay

        x: root.width + Kirigami.Units.smallSpacing
        y: (root.height - height) / 2
    }
}
