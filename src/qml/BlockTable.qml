// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true

    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    readonly property var blockData: root.block.data
    readonly property var borderColor: Qt.alpha(Kirigami.Theme.textColor, 0.5)
    readonly property var headerBgColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.3)

    property int hoveredRow: -1
    property int hoveredColumn: -1
    property int selectedRow: -1
    property int selectedColumn: -1

    Menu {
        id: rowContextMenu
        MenuItem { 
            text: "Insert Row Above" 
            onClicked: CommandManager.insertRowInTable(root.block, root.selectedRow)
        }
        MenuItem { 
            text: "Insert Row Below" 
            onClicked: CommandManager.insertRowInTable(root.block, root.selectedRow + 1)
        }
        MenuItem { 
            text: "Delete Row" 
            onClicked: CommandManager.deleteRowInTable(root.block, root.selectedRow)
        }
        onClosed: root.selectedRow = -1
    }

    Menu {
        id: colContextMenu
        MenuItem { 
            text: "Insert Column Left" 
            onClicked: CommandManager.insertColInTable(root.block, root.selectedColumn)
        }
        MenuItem { 
            text: "Insert Column Right" 
            onClicked: CommandManager.insertColInTable(root.block, root.selectedColumn + 1)
        }
        MenuItem { 
            text: "Delete Column" 
            onClicked: CommandManager.deleteColumnInTable(root.block, root.selectedColumn)
        }
        onClosed: root.selectedColumn = -1
    }

    HoverHandler {
        onHoveredChanged: {
            if (!hovered) {
                root.hoveredRow = -1;
                root.hoveredColumn = -1;
            }
        }
    }

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: scrollView.implicitHeight

        Flickable {
            id: scrollView

            implicitWidth: parent.width
            implicitHeight: tableWrapper.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)
            
            contentWidth: tableWrapper.implicitWidth
            contentHeight: tableWrapper.implicitHeight
            
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            GridLayout {
                id: tableWrapper
                columns: 3
                rowSpacing: Kirigami.Units.smallSpacing
                columnSpacing: Kirigami.Units.smallSpacing

                Item {
                    Layout.row: 0
                    Layout.column: 1
                    Layout.preferredHeight: Kirigami.Units.smallSpacing
                }

                Item {
                    Layout.row: 1
                    Layout.column: 0
                    Layout.preferredWidth: Kirigami.Units.smallSpacing
                }

                GridLayout {
                    id: table
                    
                    Layout.row: 1
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    columns: blockData.columnCount
                
                columnSpacing: 0
                rowSpacing: 0
 
                Repeater {
                    id: cellRepeater
                    model: blockData.rowCount * blockData.columnCount

                    delegate: Item {
                        id: delegateItem
                        property int rowIndex: Math.floor(index / blockData.columnCount)
                        property int columnIndex: index % blockData.columnCount

                        HoverHandler {
                            onHoveredChanged: {
                                if (hovered) {
                                    root.hoveredRow = delegateItem.rowIndex;
                                    root.hoveredColumn = delegateItem.columnIndex;
                                }
                            }
                        }

                        Layout.preferredWidth: cell.implicitWidth
                        Layout.preferredHeight: cell.implicitHeight
                        
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        BlockTableCell {
                            id: cell
                            wrapMode: TextEdit.NoWrap
                            html: blockData.htmlData[rowIndex][columnIndex]
                            md: blockData.mdData[rowIndex][columnIndex]
                            block: root.block
                            model: root.delegateModel.model
                            rowIndex: delegateItem.rowIndex
                            columnIndex: delegateItem.columnIndex
                            padding: Kirigami.Units.largeSpacing
                            color: Kirigami.Theme.textColor
                        }

                        Rectangle {
                            anchors.fill: parent
                            border.color: root.borderColor
                            border.width: 1
                            color: rowIndex === 0 ? root.headerBgColor : (rowIndex % 2 != 0 ? Kirigami.Theme.backgroundColor : Kirigami.Theme.alternateBackgroundColor)
                            z: -1
                        }

                        Rectangle {
                            visible: delegateItem.columnIndex === 0 && root.hoveredRow === delegateItem.rowIndex && root.selectedRow === -1
                            width: Kirigami.Units.gridUnit / 4.0
                            height: Kirigami.Units.gridUnit / 1.25
                            radius: Kirigami.Units.cornerRadius
                            color: Kirigami.Theme.textColor
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.left
                            z: 10
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.selectedRow = delegateItem.rowIndex;
                                    rowContextMenu.popup();
                                }
                            }
                        }

                        Rectangle {
                            visible: delegateItem.rowIndex === 0 && root.hoveredColumn === delegateItem.columnIndex && root.selectedColumn === -1
                            height: Kirigami.Units.gridUnit / 4.0
                            width: Kirigami.Units.gridUnit / 1.25
                            radius: Kirigami.Units.cornerRadius
                            color: Kirigami.Theme.textColor
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.top
                            z: 10
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.selectedColumn = delegateItem.columnIndex;
                                    colContextMenu.popup();
                                }
                            }
                        }
                    }
                }
            } // end table

            Button {
                id: colButton
                text: "+"
                Layout.row: 1
                Layout.column: 2
                Layout.fillHeight: true
                Layout.preferredWidth: Kirigami.Units.gridUnit
                
                opacity: hovered ? 1.0 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }
                onClicked: CommandManager.insertColInTable(root.block)
            }

            Button {
                id: rowButton
                text: "+"
                Layout.row: 2
                Layout.column: 1
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit
                
                opacity: hovered ? 1.0 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }
                onClicked: CommandManager.insertRowInTable(root.block)
            }
        } // end tableWrapper

        Rectangle {
            visible: root.selectedRow !== -1
            color: "transparent"
            border.color: Kirigami.Theme.highlightColor
            border.width: 2
            z: 10

            property Item firstCell: root.selectedRow !== -1 ? cellRepeater.itemAt(root.selectedRow * blockData.columnCount) : null
            x: tableWrapper.x + table.x
            y: tableWrapper.y + table.y + (firstCell ? firstCell.y : 0)
            width: table.width
            height: firstCell ? firstCell.height : 0
        }

        Rectangle {
            visible: root.selectedColumn !== -1
            color: "transparent"
            border.color: Kirigami.Theme.highlightColor
            border.width: 2
            z: 10

            property Item topCell: root.selectedColumn !== -1 ? cellRepeater.itemAt(root.selectedColumn) : null
            x: tableWrapper.x + table.x + (topCell ? topCell.x : 0)
            y: tableWrapper.y + table.y
            width: topCell ? topCell.width : 0
            height: table.height
        }
    }
}
}