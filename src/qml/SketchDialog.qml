// SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.marknote
import org.kde.ki18n

import "components"

pragma ComponentBehavior: Bound

Controls.Dialog {
    id: root

    property string notePath

    signal saved(string imagePath)

    parent: Controls.ApplicationWindow.window ? Controls.ApplicationWindow.window.overlay : null
    modal: true

    width: Math.min(900, parent.width)
    height: Math.min(600, parent.height)

    leftPadding: 1
    rightPadding: 1
    bottomPadding: 1
    topPadding: 0

    anchors.centerIn: Controls.ApplicationWindow.window ? Controls.ApplicationWindow.window.overlay : null

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

        function clear() {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.reset();

            drawing = [];
            history.reset();
            repaintRequired = true;
            canvas.requestPaint();
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
            preventStealing: true
            cursorShape: Qt.CrossCursor

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

            Controls.ButtonGroup { id: colorGroup }

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

            Controls.ToolSeparator { }

            Repeater {
                id: colorRepeater

                model: ListModel {
                    ListElement { color: "foreground" }
                    ListElement { color: "#de324c" }
                    ListElement { color: "#f4895f" }
                    ListElement { color: "#f8e16f" }
                    ListElement { color: "#95cf92" }
                    ListElement { color: "#369acc" }
                    ListElement { color: "#9656a2" }
                }

                Controls.ToolButton {
                    id: delegate
                    Controls.ButtonGroup.group: colorGroup

                    required property string color
                    required property int index

                    readonly property var foreground: Kirigami.Theme.textColor
                    readonly property string colorName: color === "foreground" ? foreground : color

                    implicitHeight: Kirigami.Units.gridUnit * 2
                    autoExclusive: true
                    checkable: true
                    checked: index === 0
                    onCheckedChanged: canvas.color = colorName
                    onForegroundChanged: {
                        if (color === "foreground" && delegate.checked) {
                            canvas.color = foreground
                        }
                    }

                    background.visible: false
                    contentItem: Item {
                        width: height
                        Kirigami.ShadowedRectangle {
                            anchors.centerIn: parent
                            color: delegate.colorName
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

            Controls.ToolSeparator { }

            Controls.ToolButton {
                id: eraserButton
                Controls.ButtonGroup.group: colorGroup
                implicitHeight: Kirigami.Units.gridUnit * 2
                autoExclusive: true
                checkable: true
                background.visible: true
                display: Controls.AbstractButton.IconOnly
                icon.name: "draw-eraser-symbolic"
                Controls.ToolTip {
                    text: KI18n.i18nc("Tool that removes selected parts of image from canvas", "Eraser")
                }
            }

            Controls.ToolButton {
                id: clearButton
                implicitHeight: Kirigami.Units.gridUnit * 2
                autoExclusive: false
                checkable: false
                background.visible: true
                enabled: true
                display: Controls.AbstractButton.IconOnly
                icon.name: "albumfolder-user-trash-symbolic"
                onClicked: canvas.clear()
                Controls.ToolTip {
                    text: KI18n.i18nc("Button that clears the canvas", "Clear Canvas")
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

            Controls.ButtonGroup { id: widthGroup }

            Repeater {
                model: ListModel {
                    ListElement { strokeWidth: 2 }
                    ListElement { strokeWidth: 4 }
                    ListElement { strokeWidth: 6 }
                    ListElement { strokeWidth: 8 }

                }
                Controls.ToolButton {
                    id: widthDelegate
                    Controls.ButtonGroup.group: widthGroup

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
                onClicked: {
                    let base = root.notePath.toString().replace(/^file:\/\//, "").replace(/\.md$/, "");
                    base = base || (StandardPaths.writableLocation(StandardPaths.TempLocation) + "/sketch");
                    const timestamp = Date.now();
                    let localFilePath = `${base}_${timestamp}.png`;

                    canvas.grabToImage(function(result) {
                        if (result.saveToFile(localFilePath)) {
                            root.saved(localFilePath);
                            root.close();
                        } else {
                            console.error("Failed to save sketch to: " + localFilePath);
                        }
                    });
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
