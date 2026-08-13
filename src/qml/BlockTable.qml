// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    readonly property bool isDarkMode: Kirigami.Theme.textColor.hslLightness > Kirigami.Theme.backgroundColor.hslLightness
    readonly property var blockData: root.block.data
    readonly property color borderColor: isDarkMode ? Qt.darker(Kirigami.Theme.textColor, 1.25) : Qt.lighter(Kirigami.Theme.textColor, 1.25)
    readonly property color headerBgColor: isDarkMode ? Qt.darker(Kirigami.Theme.highlightColor, 1.5) : Qt.lighter(Kirigami.Theme.highlightColor, 1.5)
    readonly property int borderWidth: 1 // px
    readonly property int selectionBorderWidth: 2 // px
    property int hoveredRow: -1
    property int hoveredColumn: -1
    property int selectedRow: -1
    property int selectedColumn: -1

    isFinalBlock: true
    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    Menu {
        id: rowContextMenu

        onClosed: root.selectedRow = -1

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

    }

    Menu {
        id: colContextMenu

        onClosed: root.selectedColumn = -1

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

                            Layout.preferredWidth: cell.implicitWidth
                            Layout.preferredHeight: cell.implicitHeight
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            HoverHandler {
                                onHoveredChanged: {
                                    if (hovered) {
                                        root.hoveredRow = delegateItem.rowIndex;
                                        root.hoveredColumn = delegateItem.columnIndex;
                                    }
                                }
                            }

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
                                anchors.topMargin: delegateItem.rowIndex > 0 ? -root.borderWidth : 0
                                anchors.leftMargin: delegateItem.columnIndex > 0 ? -root.borderWidth : 0
                                border.color: root.borderColor
                                border.width: root.borderWidth
                                color: rowIndex === 0 ? root.headerBgColor : (rowIndex % 2 != 0 ? Kirigami.Theme.backgroundColor : Kirigami.Theme.alternateBackgroundColor)
                                z: -1
                            }

                            Rectangle {
                                visible: delegateItem.columnIndex === 0 && root.hoveredRow === delegateItem.rowIndex && root.selectedRow === -1
                                width: Kirigami.Units.gridUnit / 4
                                height: Kirigami.Units.gridUnit / 1.25
                                radius: Kirigami.Units.cornerRadius
                                color: root.borderColor
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.left
                                z: 11

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
                                height: Kirigami.Units.gridUnit / 4
                                width: Kirigami.Units.gridUnit / 1.25
                                radius: Kirigami.Units.cornerRadius
                                color: root.borderColor
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.top
                                z: 11

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
                    // end table

                }

                Button {
                    id: colButton

                    text: "+"
                    Layout.row: 1
                    Layout.column: 2
                    Layout.fillHeight: true
                    Layout.preferredWidth: Kirigami.Units.gridUnit
                    opacity: hovered ? 1 : 0
                    onClicked: CommandManager.insertColInTable(root.block)

                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.shortDuration
                        }

                    }

                }

                Button {
                    id: rowButton

                    text: "+"
                    Layout.row: 2
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit
                    opacity: hovered ? 1 : 0
                    onClicked: CommandManager.insertRowInTable(root.block)

                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.shortDuration
                        }

                    }

                }
                // end tableWrapper

            }

            Rectangle {
                id: selectionRect

                property bool isRowSelected: root.selectedRow !== -1
                property bool isColSelected: root.selectedColumn !== -1
                property int delta: root.selectionBorderWidth - root.borderWidth
                property Item firstCell: isRowSelected ? cellRepeater.itemAt(root.selectedRow * blockData.columnCount) : null
                property Item topCell: isColSelected ? cellRepeater.itemAt(root.selectedColumn) : null

                visible: isRowSelected || isColSelected
                color: "transparent"
                border.color: Kirigami.Theme.highlightColor
                border.width: root.selectionBorderWidth
                z: 10
                x: tableWrapper.x + table.x - delta + (topCell ? topCell.x - (root.selectedColumn > 0 ? root.borderWidth : 0) : 0)
                y: tableWrapper.y + table.y - delta + (firstCell ? firstCell.y - (root.selectedRow > 0 ? root.borderWidth : 0) : 0)
                width: (isColSelected && topCell ? topCell.width + (root.selectedColumn > 0 ? root.borderWidth : 0) : table.width) + 2 * delta
                height: (isRowSelected && firstCell ? firstCell.height + (root.selectedRow > 0 ? root.borderWidth : 0) : table.height) + 2 * delta
            }

            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

        }

    }

}
