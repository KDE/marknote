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
    required property var blockData;
    required property bool isFinalBlock;
    property int topMargin: 0
    property int bottomMargin: 0

    implicitWidth: ListView.view ? ListView.view.width : 0
    implicitHeight: row.implicitHeight + root.topMargin + root.bottomMargin

    property var parentModel: ListView.view ? ListView.view.model : null
    property var cppModel: parentModel ? parentModel.model : null
    property var nodeIndex: parentModel ? parentModel.modelIndex(index) : null

    property Component blockComponent: null;

    radius: Kirigami.Units.smallSpacing
    color: hoverHandler.hovered ? Qt.alpha(Kirigami.Theme.textColor, 0.2) : "transparent"

    HoverHandler {
        id: hoverHandler
        target: root
        blocking: true
    }

    RowLayout {
        id: row

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Kirigami.Units.mediumSpacing
        anchors.rightMargin: Kirigami.Units.mediumSpacing
        anchors.topMargin: root.topMargin
        anchors.bottomMargin: root.bottomMargin
        anchors.verticalCenter: parent.verticalCenter

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

                    delegate: root.parentModel ? root.parentModel.delegate : null 
                }
            }
        }
    }
}