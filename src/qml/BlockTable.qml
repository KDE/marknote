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

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: scrollView.implicitHeight

        Flickable {
            id: scrollView

            implicitWidth: parent.width
            implicitHeight: table.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)
            
            contentWidth: table.implicitWidth
            contentHeight: table.implicitHeight
            
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            GridLayout {
                id: table
                
                columns: blockData.columnCount
                
                columnSpacing: 0
                rowSpacing: 0
 
                Repeater {
                    model: blockData.rowCount * blockData.columnCount

                    delegate: Item {
                        property int rowIndex: Math.floor(index / blockData.columnCount)
                        property int columnIndex: index % blockData.columnCount

                        Layout.preferredWidth: cell.implicitWidth
                        Layout.preferredHeight: cell.implicitHeight
                        
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            id: cell
                            text: blockData.htmlData[rowIndex][columnIndex]
                            textFormat: Text.RichText
                            padding: Kirigami.Units.smallSpacing 
                            font.bold: rowIndex === 0
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
        }
    }
}