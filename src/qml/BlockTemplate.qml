// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models

import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    required property var index;
    required property var block;
    required property bool isFinalBlock;
    property int topMargin: 0
    property int bottomMargin: 0

    implicitWidth: ListView.view ? ListView.view.width : 0
    implicitHeight: row.implicitHeight + root.topMargin + root.bottomMargin

    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    readonly property bool isDarkMode: sysPalette.windowText.hslLightness > sysPalette.window.hslLightness
    property var delegateModel: ListView.view ? ListView.view.model : null
    property var cppModel: delegateModel ? delegateModel.model : null
    property var nodeIndex: delegateModel ? delegateModel.modelIndex(index) : null

    property Component blockComponent: null;

    property bool isSelected: ListView.view && ListView.view.selectedIndices ? ListView.view.selectedIndices.includes(index) : false

    radius: Kirigami.Units.smallSpacing
    color: isSelected ? Qt.alpha(Kirigami.Theme.highlightColor, 0.2) : "transparent"
    Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }

    HoverHandler {
        id: blockHoverHandler
        blocking: true
    }

    Rectangle {
        id: dragHandle
        
        anchors.right: row.left
        anchors.rightMargin: Kirigami.Units.mediumSpacing
        anchors.verticalCenter: root.verticalCenter
        
        implicitWidth: handleText.implicitWidth + Kirigami.Units.smallSpacing * 2
        implicitHeight: root.height
        radius: Kirigami.Units.cornerRadius
        color: isDarkMode ? Qt.darker(Kirigami.Theme.textColor, 3) : Qt.lighter(Kirigami.Theme.textColor, 3)

        opacity: dragMouseArea.containsMouse ? 1.0 : (blockHoverHandler.hovered ? 0.2 : 0)
        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        z: 99

        Text {
            id: handleText
            text: "\u205D\u205D"
            color: Kirigami.Theme.textColor
            anchors.centerIn: parent
        }

        MouseArea {
            id: dragMouseArea
            anchors.left: dragHandle.left
            anchors.top: dragHandle.top
            anchors.bottom: dragHandle.bottom
            width: dragHandle.implicitWidth * 2
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
            onPressed: cursorShape = Qt.ClosedHandCursor
            onReleased: cursorShape = Qt.OpenHandCursor
        }
    }

    RowLayout {
        id: row

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: dragMouseArea.containsMouse ? Kirigami.Units.largeSpacing * 2 : Kirigami.Units.mediumSpacing
        anchors.rightMargin: Kirigami.Units.mediumSpacing
        anchors.topMargin: root.topMargin
        anchors.bottomMargin: root.bottomMargin
        anchors.verticalCenter: parent.verticalCenter

        Behavior on anchors.leftMargin {
            SequentialAnimation {
                PauseAnimation {
                    duration: Kirigami.Units.longDuration
                }
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                }
            }
        }

        Loader {
            id: blockLoader

            Layout.fillHeight: !root.isFinalBlock
            Layout.preferredHeight: root.isFinalBlock ? -1 : 0

            Layout.fillWidth: root.isFinalBlock;
            visible: blockComponent !== null

            sourceComponent: blockComponent
        }

        Loader {
            id: childLoader
            active: !root.isFinalBlock
            visible: active

            Layout.fillWidth: true

            sourceComponent: ListView {
                id: childWrapper

                implicitHeight: contentHeight
                interactive: false

                model: DelegateModel {
                    id: childDelegateModel
                    model: root.cppModel
                    rootIndex: root.nodeIndex

                    delegate: root.delegateModel ? root.delegateModel.delegate : null 
                }
            }
        }
    }
}