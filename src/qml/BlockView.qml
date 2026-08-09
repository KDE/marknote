// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models

import org.kde.kirigami as Kirigami

Item {
    id: root
    anchors.fill: parent

    required property var richDocumentHandler;

    property alias listView: blockListView

    Shortcut {
        sequence: StandardKey.SelectAll
        onActivated: {
            if (richDocumentHandler && richDocumentHandler.treeModel) {
                richDocumentHandler.treeModel.selectAll();
            }
        }
    }

    property real selectionStartContentX: 0
    property real selectionStartContentY: 0
    property real selectionCurrentContentX: 0
    property real selectionCurrentContentY: 0
    property bool isSelecting: false
    
    property int selectionStartIndex: -1
    property int selectionCurrentIndex: -1

    function getIndexAtContentY(yPos) {
        let children = blockListView.contentItem.children;
        let closestIndex = -1;
        let minDistance = Infinity;
        
        for (let i = 0; i < children.length; ++i) {
            let child = children[i];
            if (child.index !== undefined) {
                if (yPos >= child.y && yPos <= child.y + child.height) {
                    return child.index;
                }
                
                let distToTop = Math.abs(yPos - child.y);
                let distToBottom = Math.abs(yPos - (child.y + child.height));
                let dist = Math.min(distToTop, distToBottom);
                
                if (dist < minDistance) {
                    minDistance = dist;
                    closestIndex = child.index;
                }
            }
        }
        return closestIndex;
    }

    function updateSelection() {
        if (!isSelecting) return;
        
        selectionCurrentIndex = getIndexAtContentY(selectionCurrentContentY);
        
        if (selectionStartIndex !== -1 && selectionCurrentIndex !== -1) {
            let start = Math.min(selectionStartIndex, selectionCurrentIndex);
            let end = Math.max(selectionStartIndex, selectionCurrentIndex);
            
            let newSelected = [];
            for (let i = start; i <= end; ++i) {
                newSelected.push(i);
            }
            richDocumentHandler.treeModel.selectedIndices = newSelected;
        }
    }

    MouseArea {
        parent: root.Overlay.overlay
        anchors.fill: parent
        visible: richDocumentHandler.treeModel && richDocumentHandler.treeModel.selectedIndices.length > 0
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        propagateComposedEvents: true
        preventStealing: false
        onPressed: (mouse) => {
            richDocumentHandler.treeModel.selectedIndices = [];
            mouse.accepted = false; 
        }
    }

    Timer {
        id: autoScrollTimer
        interval: 16
        repeat: true
        running: root.isSelecting
        onTriggered: {
            let localY = selectionDragHandler.centroid.position.y;
            let edgeMargin = 40;
            let scrollSpeed = 0;

            if (localY < edgeMargin) {
                scrollSpeed = (localY - edgeMargin) * 0.5;
            } else if (localY > root.height - edgeMargin) {
                scrollSpeed = (localY - (root.height - edgeMargin)) * 0.5;
            }

            if (scrollSpeed !== 0) {
                let minContentY = blockListView.originY;
                let maxContentY = Math.max(minContentY, blockListView.contentHeight - blockListView.height + blockListView.originY);
                
                let newContentY = blockListView.contentY + scrollSpeed;
                if (newContentY < minContentY) newContentY = minContentY;
                if (newContentY > maxContentY) newContentY = maxContentY;
                
                if (newContentY !== blockListView.contentY) {
                    blockListView.contentY = newContentY;
                    root.selectionCurrentContentY = localY + blockListView.contentY;
                    root.updateSelection();
                }
            }
        }
    }

    DragHandler {
        id: selectionDragHandler
        target: null
        onActiveChanged: {
            if (active) {
                root.isSelecting = true;
                root.selectionStartContentX = centroid.position.x - blockListView.x + blockListView.contentX;
                root.selectionStartContentY = centroid.position.y - blockListView.y + blockListView.contentY;
                root.selectionCurrentContentX = centroid.position.x - blockListView.x + blockListView.contentX;
                root.selectionCurrentContentY = centroid.position.y - blockListView.y + blockListView.contentY;
                
                root.selectionStartIndex = root.getIndexAtContentY(root.selectionStartContentY);
                root.selectionCurrentIndex = root.selectionStartIndex;
                
                blockListView.interactive = false;
            } else {
                root.isSelecting = false;
                blockListView.interactive = true;
            }
        }
        onCentroidChanged: {
            if (active) {
                root.selectionCurrentContentX = centroid.position.x - blockListView.x + blockListView.contentX;
                root.selectionCurrentContentY = centroid.position.y - blockListView.y + blockListView.contentY;
                root.updateSelection();
            }
        }
    }

    Rectangle {
        visible: root.isSelecting
        x: Math.min(root.selectionStartContentX, root.selectionCurrentContentX) - blockListView.contentX + blockListView.x
        y: Math.min(root.selectionStartContentY, root.selectionCurrentContentY) - blockListView.contentY + blockListView.y
        width: Math.abs(root.selectionCurrentContentX - root.selectionStartContentX)
        height: Math.abs(root.selectionCurrentContentY - root.selectionStartContentY)
        color: Qt.alpha(Kirigami.Theme.highlightColor, 0.3)
        border.color: Kirigami.Theme.highlightColor
        border.width: 1
        z: 99
    }

    DelegateModel {
        id: treeDelegateModel
        model: richDocumentHandler.treeModel

        delegate: BlockChooser { }
    }

    RowLayout {
        anchors.fill: parent

        ListView {
            id: blockListView
            property var selectedIndices: richDocumentHandler.treeModel ? richDocumentHandler.treeModel.selectedIndices : []
            model: treeDelegateModel
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: verticalScrollBar
            synchronousDrag: true

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing

            bottomMargin: parent.height / 2.0
        }

        ScrollBar {
            id: verticalScrollBar
            policy: ScrollBar.AlwaysOn
            Layout.fillHeight: true
        }
    }
}
