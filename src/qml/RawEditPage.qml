// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts

import "components"

import org.kde.kirigamiaddons.components as Components
import org.kde.marknote

Kirigami.Page {
    id: root

    property
     alias document: document

    property bool init: false

    property string noteFullPath: NavigationController.noteFullPath

    // Only overwrite these values in MainEditor
    property string noteName: NavigationController.noteName
    property string oldPath: ''
    property bool saved: true
    property bool singleDocumentMode: false
    // readonly property bool wideScreen: width >= toolBar.width + Kirigami.Units.largeSpacing * 2
    readonly property bool wideScreen: true

    function loadNote(): void {
        if (root.oldPath.length > 0 && !saved) {
            document.saveAs(root.oldPath);
        }
        if (root.noteFullPath.toString().length > 0) {
            document.load(root.noteFullPath);
            root.saved = true;
        }
        root.oldPath = root.noteFullPath;

        textArea.forceActiveFocus();
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false
    Layout.fillWidth: true
    bottomPadding: 0
    leftPadding: 0
    objectName: "RawEditPage"
    rightPadding: 0
    topPadding: 0

    contentItem: ScrollView {
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        T.TextArea {
            id: textArea

            // To eliminate text overlap by the textFormatGroup we introduce extra padding
            readonly property int additionalPadding: Kirigami.Units.gridUnit * 4
            // property int lastKey: -1

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: background == null
            background: null
            bottomPadding: additionalPadding
            color: Kirigami.Theme.textColor
            font: Config.editorFont
            height: parent.height
            implicitHeight: Math.max(contentHeight + topPadding + bottomPadding, implicitBackgroundHeight + topInset + bottomInset)
            implicitWidth: Math.max(contentWidth + leftPadding + rightPadding, implicitBackgroundWidth + leftInset + rightInset)
            leftPadding: 0
            persistentSelection: true
            placeholderTextColor: Kirigami.Theme.disabledTextColor
            rightPadding: 0
            selectByMouse: true
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            selectionColor: Kirigami.Theme.highlightColor
            textFormat: TextEdit.PlainText
            textMargin: wideScreen ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 1
            topPadding: 0
            wrapMode: TextEdit.Wrap

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            // Keys.onPressed: event => {
            //     lastKey = event.key;
            // }
            onPressAndHold: {
                if (Kirigami.Settings.tabletMode && selectByMouse) {
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }
            onTextChanged: {
                // if (lastKey !== -1) {
                //     let key = lastKey;
                //     lastKey = -1;
                //     document.slotKeyPressed(key);
                // }
                root.saved = false;
                saveTimer.restart();
            }

            DropArea {
                id: imageDropArea

                anchors.fill: parent
                keys: ["text/uri-list"]

                onDropped: drop => {
                    if (drop.hasUrls) {
                        const path = drop.urls[0].toString();
                        const isImage = /\.(png|jpg|jpeg|svg|webp|avif)$/i.test(path);

                        if (isImage) {
                            document.insertImage(path);
                        } else {
                            console.warn("Dropped URL is not a supported image format:", path);
                        }
                    }
                }
                onEntered: drag => {
                    if (drag.hasUrls || drag.hasText) {
                        drag.acceptProposedAction();
                    }
                }

                RawDocumentHandler { // need to change this to the new handler
                    id: document

                    cursorPosition: textArea.cursorPosition
                    document: textArea.textDocument
                    selectionEnd: textArea.selectionEnd
                    textArea: textArea

                    Component.onCompleted: {
                        if (root.noteFullPath.toString().length > 0) {
                            document.load(root.noteFullPath);
                            root.saved = true;
                            root.oldPath = root.noteFullPath;
                            textArea.forceActiveFocus();
                        }
                    }
                    Component.onDestruction: {
                        if (!saved && root.noteFullPath.toString().length > 0) {
                            document.saveAs(root.noteFullPath);
                        }
                    }

                    onCopy: textArea.copy()

                    onCut: textArea.cut()
                    onError: message => {
                        console.error("Error message from document handler", message);
                    }
                    // textColor: TODO
                    onLoaded: text => {
                        textArea.text = text;
                    }
                    onMoveCursor: position => {
                        textArea.cursorPosition = position;
                    }
                    onRedo: textArea.redo()
                    onSelectCursor: (start, end) => {
                        textArea.select(start, end);
                    }
                    onUndo: textArea.undo()
                }
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                // unfortunately, taphandler's pressed event only triggers when the press is lifted
                // we need to use the longpress signal since it triggers when the button is first pressed
                longPressThreshold: 0.001 // https://invent.kde.org/qt/qt/qtdeclarative/-/commit/8f6809681ec82da783ae8dcd76fa2c209b28fde6

                onLongPressed: {
                    textFieldContextMenu.currentLink = document.anchorAt(point.position);
                    textFieldContextMenu.targetClick(point, textArea,
                    /*spellcheckHighlighterInstantiator*/ null,
                    /*mousePosition*/ null);
                }
            }
            Timer {
                id: saveTimer

                interval: 1000
                repeat: false

                onTriggered: if (root.noteFullPath.toString().length > 0) {
                    document.saveAs(root.noteFullPath);
                    saved = true;
                }
            }
        }
    }
    titleDelegate: RowLayout {
        Layout.fillWidth: true
        visible: root.noteName

        ToolButton {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canUndo
            icon.name: "edit-undo"
            text: i18n("Undo")
            visible: wideScreen

            onClicked: textArea.undo()
        }
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canRedo
            icon.name: "edit-redo"
            text: i18n("Redo")
            visible: wideScreen

            onClicked: textArea.redo()
        }
        Item {
            Layout.fillWidth: true
        }
        Rectangle {
            color: Kirigami.Theme.textColor
            height: 5
            radius: 2.5
            scale: root.saved ? 0 : 1
            width: height

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
        }
        Kirigami.Heading {
            Layout.leftMargin: Kirigami.Units.mediumSpacing
            Layout.rightMargin: Kirigami.Units.mediumSpacing
            text: root.noteName
        }
        Item {
            width: 5
        }
        Item {
            Layout.fillWidth: true
        }
        Item {
            visible: wideScreen
            width: fillWindowButton.width
        }

        RowLayout {
            spacing: 5

            Label {
                opacity: toggleSwitch.checked ? 0.5 : 1.0
                text: "Source"
            }
            Switch {
                id: toggleSwitch
                checked: false
                onToggled: {
                    NavigationController.sourceMode = !NavigationController.sourceMode
                }
            }
            Label {
                opacity: toggleSwitch.checked ? 1.0 : 0.5
                text: "Preview"
            }
        }

        ToolButton {
            id: fillWindowButton

            property int columnWidth: Config.fillWindow ? 0 : Kirigami.Units.gridUnit * 15

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            checkable: true
            checked: Config.fillWindow
            display: AbstractButton.IconOnly
            icon.name: "view-fullscreen"
            text: i18n("Focus Mode")
            visible: wideScreen && !root.singleDocumentMode

            Behavior on columnWidth {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            onClicked: {
                Config.fillWindow = !Config.fillWindow;
            }
            onColumnWidthChanged: pageStack.defaultColumnWidth = columnWidth

            Shortcut {
                sequence: "Ctrl+R"

                onActivated: Config.fillWindow = !Config.fillWindow
            }
        }                                                 
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            checkable: true
            checked: true
            display: AbstractButton.IconOnly
            icon.name: "window-restore-symbolic"
            text: i18nc("@action:menu", "Exit Full Screen")
            visible: applicationWindow().visibility === Window.FullScreen

            onClicked: applicationWindow().showNormal()
        }
    }

    Component.onCompleted: {
        init = true;
        loadNote();
    }
    onNoteFullPathChanged: () => {
        if (!init) {
            return;
        }
        loadNote();
    }

}
