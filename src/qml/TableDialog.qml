// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtCore
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs

FormCard.FormCardDialog {
    id: root

    readonly property alias rows: rowsSpinBox.value
    readonly property alias cols: colsSpinBox.value

    title: i18nc("@title:window", "Insert Table")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    onAccepted: close();
    onRejected: close();
    MouseArea {
        Layout.alignment: Qt.AlignHCenter
        width: grid.width
        height: grid.height

        hoverEnabled: true
        preventStealing: true

        Layout.topMargin: Kirigami.Units.largeSpacing * 2
        Layout.bottomMargin: Kirigami.Units.largeSpacing

        onEntered: grid.hovered = true
        onExited:  grid.hovered = false

        GridLayout {
            id: grid

            anchors.centerIn: parent

            property int hoveredXPos: 0
            property int hoveredYPos: 0

            property int clickedXPos: 0
            property int clickedYPos: 0

            onClickedXPosChanged: print(clickedXPos, clickedYPos, "clicked")

            property bool hovered: true
            property bool clicked: false



            columns: 11
            rows: 11
            columnSpacing: 0
            rowSpacing: 0
            Repeater {
                model: grid.columns * grid.rows
                delegate: MouseArea {
                    id: delegate
                    required property int index
                    property int gridXPos: index % grid.columns + 1
                    property int gridYPos: ~~(index / grid.columns) + 1
                    hoverEnabled: true

                    height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing
                    width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing

                    preventStealing: true

                    onEntered: {
                        grid.hoveredXPos = delegate.gridXPos
                        grid.hoveredYPos = delegate.gridYPos
                    }
                    onPressed: {
                        grid.clickedXPos = gridXPos
                        grid.clickedYPos = gridYPos
                        grid.clicked = true
                        print(gridXPos, gridYPos)
                    }
                    Kirigami.ShadowedRectangle {
                        height: Kirigami.Units.gridUnit
                        width: Kirigami.Units.gridUnit
                        anchors.centerIn: parent
                        Kirigami.Theme.colorSet: Kirigami.Theme.Button
                        corners {
                            topLeftRadius: (delegate.gridXPos === 1 && delegate.gridYPos === 1)? Kirigami.Units.cornerRadius: 0
                            topRightRadius: (delegate.gridXPos === grid.columns && delegate.gridYPos === 1)? Kirigami.Units.cornerRadius: 0
                            bottomLeftRadius: (delegate.gridXPos === 1 && delegate.gridYPos === grid.rows)? Kirigami.Units.cornerRadius: 0
                            bottomRightRadius: (delegate.gridXPos === grid.columns && delegate.gridYPos === grid.rows)? Kirigami.Units.cornerRadius: 0
                        }

                        color: if ((delegate.gridXPos <= grid.hoveredXPos && delegate.gridYPos <= grid.hoveredYPos && grid.hovered) ) {
                                   Kirigami.Theme.hoverColor
                               } else if (delegate.gridXPos <= grid.clickedXPos && delegate.gridYPos <= grid.clickedYPos) {
                                   Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.7)
                               } else {
                                   Kirigami.Theme.backgroundColor
                               }
                        Behavior on color {
                            ColorAnimation {
                                duration: Kirigami.Units.shortDuration * 0.5
                            }
                        }
                    }
                }
            }
        }

    }

    RowLayout {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2

        Layout.alignment: Qt.AlignHCenter
        QQC2.SpinBox {
            id: rowsSpinBox

            from: 1
            value: grid.hovered? grid.hoveredXPos : grid.clickedXPos
            onValueModified: grid.clickedXPos = value
        }

        QQC2.Label {
            text: "×"
        }

        QQC2.SpinBox {
            id: colsSpinBox
            from: 1
            value: grid.hovered? grid.hoveredYPos : grid.clickedYPos
            onValueModified: grid.clickedYPos = value

        }
    }
}
