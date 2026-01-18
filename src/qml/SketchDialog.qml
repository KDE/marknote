// SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import QtQuick.Templates as T
import org.kde.marknote

import "components"

Controls.Dialog {
    id: root

    property string notePath
    property alias imagePath: saveButton.imagePath

    parent: applicationWindow().overlay
    modal: true

    width: Math.min(900, parent.width)
    height: 600

    leftPadding: 1
    rightPadding: 1
    bottomPadding: 1
    topPadding: 0

    anchors.centerIn: applicationWindow().overlay

    background: Components.DialogRoundedBackground {}

    Canvas {
        id: canvas
        anchors.fill: parent
        property real lastX
        property real lastY
        property color color: "black"
        property int strokeWidth: 2
        property bool erase: eraserButton.checked
        property bool mousePressed: false;
        property var drawing: []; // array of stroke objects
        property var currentStroke: ({
            points: [],
            color: "",
            width: 0,
            isEraser: false
        }); // stroke object that contains points, color, width
        property bool repaintRequired: false;

        function undo(): void {
            if (mousePressed) {
                return;
            }
            if (history.undoAvailable){
                history.undoStroke();
                drawing.pop();
                canvas.repaintRequired = true;
                canvas.requestPaint();
            }
        }

        function redo(): void {
            if (mousePressed) {
                return;
            }
            if (history.redoAvailable) {
                let cppStroke = history.redoStroke();
                let jsStroke = {
                    points: cppStroke.points,
                    color: cppStroke.color.toString(),
                    width: cppStroke.width,
                    isEraser: cppStroke.isEraser
                }
                drawing.push(jsStroke);
                canvas.repaintRequired = true;
                canvas.requestPaint();
            }
        }

        function makeStroke(): void {
            if (mousePressed) {
                return;
            }
            history.submitStroke(currentStroke.points, currentStroke.color, currentStroke.width, currentStroke.isEraser);
            drawing.push(currentStroke);
        }

        HistoryController {
            id: history
        }

        onPaint: {
            var ctx = getContext('2d')
            if (!repaintRequired) {
                if (canvas.erase === true) {
                    ctx.globalCompositeOperation = 'destination-out'
                }
                else {
                    ctx.globalCompositeOperation = 'source-over'
                }
                ctx.lineWidth = canvas.strokeWidth
                ctx.strokeStyle = canvas.color
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.moveTo(lastX, lastY)
                currentStroke.points.push(Qt.vector2d(lastX, lastY))
                lastX = area.mouseX
                lastY = area.mouseY
                ctx.lineTo(lastX, lastY)
                ctx.stroke()
            }
            else{
                ctx.reset();
                ctx.lineCap = "round"
                for (const stroke of drawing){
                    ctx.globalCompositeOperation = stroke.isEraser === true ? 'destination-out' : 'source-over';
                    ctx.lineWidth = stroke.width
                    ctx.strokeStyle = stroke.color
                    for (let i=0; i<stroke.points.length-1; i++){
                        ctx.beginPath();
                        ctx.moveTo(stroke.points[i].x, stroke.points[i].y);
                        ctx.lineTo(stroke.points[i+1].x, stroke.points[i+1].y);
                        ctx.stroke();
                    }
                }
            }

        }

        MouseArea {
            id: area
            anchors.fill: parent
            onPressed: {
                canvas.mousePressed = true;
                canvas.lastX = mouseX
                canvas.lastY = mouseY
                canvas.currentStroke = {
                    points: [],
                    color: canvas.color.toString(),
                    width: canvas.strokeWidth,
                    isEraser: canvas.erase
                }
            }
            onReleased: {
                canvas.mousePressed = false;
                canvas.currentStroke.points.push(Qt.vector2d(canvas.lastX, canvas.lastY));
                canvas.makeStroke();
            }
            onPositionChanged: {
                canvas.repaintRequired = false;
                canvas.requestPaint()
            }
        }
    }

    Components.FloatingToolBar {
        id: colorToolBarContainer

        z: 600000

        anchors {
            top: parent.top
            margins: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        contentItem: RowLayout {
            id: colorLayout

            Controls.ButtonGroup {
                buttons: colorLayout.children
            }

            Controls.ToolButton {
                id: undoButton
                implicitHeight: Kirigami.Units.gridUnit * 2
                autoExclusive: false
                checkable: false
                background.visible: true
                enabled: history.undoAvailable;
                display: Controls.AbstractButton.IconOnly
                icon.name: "edit-undo-symbolic"
                onClicked: canvas.undo()
            }

            Controls.ToolButton {
                id: redoButton
                implicitHeight: Kirigami.Units.gridUnit * 2
                autoExclusive: false
                checkable: false
                background.visible: true
                enabled: history.redoAvailable;
                display: Controls.AbstractButton.IconOnly
                icon.name: "edit-redo-symbolic"
                onClicked: canvas.redo()
            }

            Controls.ToolButton {
                id: eraserButton
                implicitHeight: Kirigami.Units.gridUnit * 2
                autoExclusive: true
                checkable: true
                background.visible: false
                contentItem: Item {
                    width: height
                    Kirigami.ShadowedRectangle {
                        anchors.centerIn: parent
                        color: "white"
                        radius: Kirigami.Units.mediumSpacing
                        width: eraserButton.checked ? parent.width - 10 : parent.width  - 4
                        height: width
                        border{
                            width: 1
                            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                        }
                        shadow {
                            size: 5
                            yOffset: 3
                            color: Qt.rgba(0, 0, 0, 0.2)
                        }

                        Behavior on width {
                            NumberAnimation {
                                duration: Kirigami.Units.shortDuration
                                easing.type: Easing.InOutQuart
                            }
                        }
                    }
                }
            }

            Repeater {
                model: ListModel {
                    ListElement { color: "#1A1B1D" }
                    ListElement { color: "#de324c" }
                    ListElement { color: "#f4895f" }
                    ListElement { color: "#f8e16f" }
                    ListElement { color: "#95cf92" }
                    ListElement { color: "#369acc" }
                    ListElement { color: "#9656a2" }
                }
                Controls.ToolButton {
                    id: delegate
                    implicitHeight: Kirigami.Units.gridUnit * 2
                    autoExclusive: true
                    checkable: true
                    onClicked: canvas.color = color
                    required property string color
                    background.visible: false
                    contentItem: Item {
                        width: height
                        Kirigami.ShadowedRectangle {
                            anchors.centerIn: parent
                            color: delegate.color
                            radius: Kirigami.Units.mediumSpacing
                            width: delegate.checked ? parent.width - 10 : parent.width  - 4
                            height: width
                            border{
                                width: 1
                                color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                            }
                            shadow {
                                size: 5
                                yOffset: 3
                                color: Qt.rgba(0, 0, 0, 0.2)
                            }

                            Behavior on width {
                                NumberAnimation {
                                    duration: Kirigami.Units.shortDuration
                                    easing.type: Easing.InOutQuart
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Components.FloatingToolBar {
        id: widthToolBarContainer

        z: 600000

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            margins: Kirigami.Units.largeSpacing
        }

        contentItem: ColumnLayout {
            id: widthLayout

            Controls.ButtonGroup {
                buttons: widthLayout.children
            }

            Repeater {
                model: ListModel {
                    ListElement { strokeWidth: 2 }
                    ListElement { strokeWidth: 4 }
                    ListElement { strokeWidth: 6 }
                    ListElement { strokeWidth: 8 }

                }
                Controls.ToolButton {
                    id: widthDelegate
                    implicitHeight: Kirigami.Units.gridUnit * 2
                    autoExclusive: true
                    checkable: true
                    onClicked: canvas.strokeWidth = strokeWidth
                    required property int strokeWidth
                    contentItem: Item {
                        width: height
                        Kirigami.ShadowedRectangle {
                            anchors.centerIn: parent
                            color: Kirigami.Theme.textColor
                            radius: 200
                            width: widthDelegate.strokeWidth * 2
                            height: width

                            Behavior on width {
                                NumberAnimation {
                                    duration: Kirigami.Units.shortDuration
                                    easing.type: Easing.InOutQuart
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Components.FloatingToolBar {
        id: bottomToolBarContainer

        z: 600000

        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: Kirigami.Units.largeSpacing
        }

        contentItem: RowLayout{
            Controls.ToolButton {
                text: "Cancel"
                icon.name: "dialog-close"
                onClicked:{
                    root.close()
                }
            }
            Controls.ToolButton {
                id: saveButton
                property string imagePath
                text: "Save"
                icon.name: "answer-correct"
                onClicked:{
                    var notepath = root.notePath.slice(7, -3)/*.replace(" ", "_")*/
                    imagePath = notepath + Math.random() * 1000 +".png"
                    print(canvas.save(imagePath))
                    root.close()
                }
            }
        }
    }

    onClosed: {
        history.reset();
        canvas.drawing = [];
        canvas.repaintRequired = true;
        canvas.requestPaint();
    }
}
