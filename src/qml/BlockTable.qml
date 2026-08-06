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
                    model: blockData.rowCount * blockData.columnCount

                    delegate: Item {
                        id: delegateItem
                        property int rowIndex: Math.floor(index / blockData.columnCount)
                        property int columnIndex: index % blockData.columnCount

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
                            fontBold: rowIndex === 0 ? true : Kirigami.Theme.defaultFont.bold
                            color: rowIndex === 0 ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                        }

                        Rectangle {
                            anchors.fill: parent
                            border.color: Kirigami.Theme.textColor
                            border.width: 1
                            color: rowIndex === 0 ? Kirigami.Theme.highlightColor : (rowIndex % 2 != 0 ? Kirigami.Theme.backgroundColor : Kirigami.Theme.alternateBackgroundColor)
                            z: -1
                        }
                    }
                }
            }

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
        }
    }
}
}