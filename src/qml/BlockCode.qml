// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.smallSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    readonly property var blockData: root.block.data
    readonly property var text: blockData.text

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: scrollView.implicitHeight
        
        Flickable {
            id: scrollView

            implicitWidth: parent.width
            implicitHeight: codeText.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)

            contentWidth: codeText.width
            contentHeight: codeText.implicitHeight
            
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            TextArea {
                id: codeText
                text: blockData.text
                font.family: Kirigami.Theme.fixedWidthFont.family
                color: Kirigami.Theme.textColor
                padding: Kirigami.Units.largeSpacing

                width: Math.max(implicitWidth, scrollView.width)
                implicitHeight: Math.max(buttonRow.implicitHeight + buttonRow.anchors.topMargin * 2, contentHeight + topPadding + bottomPadding)

                background: Rectangle {
                    color: Qt.alpha(Kirigami.Theme.textColor, 0.1)
                    radius: Kirigami.Units.smallSpacing
                }

                property string lastSavedText: root.blockData.text
                property int lastSavedCursorPos: 0
                property bool isProgrammaticUpdate: false

                Timer {
                    id: editTimer
                    interval: 500
                    repeat: false
                    onTriggered: {
                        if (codeText.lastSavedText !== codeText.text) {
                            CommandManager.editCode(root.block, codeText.lastSavedText, codeText.text, codeText.lastSavedCursorPos, codeText.cursorPosition);
                            codeText.lastSavedText = codeText.text;
                            codeText.lastSavedCursorPos = codeText.cursorPosition;
                        }
                    }
                }

                function flushTimer() {
                    if (editTimer.running) {
                        editTimer.stop();
                        editTimer.triggered();
                    }
                }

                onTextChanged: {
                    if (isProgrammaticUpdate) return;
                    if (codeText.activeFocus && codeText.text !== codeText.lastSavedText) {
                        editTimer.restart();
                    }
                }

                Keys.onReturnPressed: event => {
                    if (event.modifiers & Qt.ShiftModifier) {
                        editTimer.stop();
                        CommandManager.splitCode(root.block, codeText.lastSavedText, codeText.text, codeText.cursorPosition);
                        codeText.lastSavedText = codeText.text.substring(0, codeText.cursorPosition);
                        event.accepted = true;
                    } else {
                        flushTimer();
                        event.accepted = false;
                    }
                }

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                        flushTimer();
                        if (event.modifiers & Qt.ShiftModifier) {
                            CommandManager.redo();
                        } else {
                            CommandManager.undo();
                        }
                        event.accepted = true;
                    }

                    if (event.key === Qt.Key_Up) {
                        let textBeforeCursor = codeText.text.substring(0, codeText.cursorPosition);
                        if (textBeforeCursor.indexOf('\n') === -1) {
                            CommandManager.moveToPreviousBlock(root.block, codeText.text, codeText.cursorPosition);
                            flushTimer();
                            event.accepted = true;
                        }
                    }

                    if (event.key === Qt.Key_Down) {
                        let textAfterCursor = codeText.text.substring(codeText.cursorPosition);
                        if (textAfterCursor.indexOf('\n') === -1) {
                            CommandManager.moveToNextBlock(root.block, codeText.text, codeText.cursorPosition);
                            flushTimer();
                            event.accepted = true;
                        }
                    }
                }

                onActiveFocusChanged: {
                    if (!activeFocus) {
                        if (root.cppModel && root.cppModel.focusedBlock() === root.block) {
                            // Focus lost due to some unknown reason
                            // We need to bring it back
                            Qt.callLater(() => {
                                root.cppModel.requestFocus(root.block, root.cppModel.focusedBlockCursorPos());
                            })
                            return;
                        }
                        flushTimer();
                    } else {
                        codeText.lastSavedCursorPos = codeText.cursorPosition;
                        if (root.cppModel) {
                            root.cppModel.setFocusedBlock(root.block, codeText.cursorPosition);
                        }
                    }
                }

                Connections {
                    target: root.cppModel

                    function onFocusRequested(requestedBlock, cursorPosition) {
                        if (requestedBlock === root.block) {
                            if (cursorPosition >= 0) {
                                codeText.cursorPosition = cursorPosition;
                            }
                            codeText.forceActiveFocus();
                        }
                    }
                }

                Connections {
                    target: root

                    function onTextChanged() {
                        codeText.lastSavedText = root.text
                        if (codeText.text !== root.text) {
                            codeText.isProgrammaticUpdate = true;
                            codeText.text = root.text;
                            codeText.isProgrammaticUpdate = false;
                        }
                    }
                }

                Component.onCompleted: {
                    Qt.callLater(() => {
                        if (root.cppModel && root.block === root.cppModel.focusedBlock()) {
                            codeText.cursorPosition = root.cppModel.focusedBlockCursorPos();
                            codeText.forceActiveFocus();
                        }
                    })
                }
            }

            MouseArea {
                anchors.fill: codeText
                enabled: !codeText.activeFocus
                cursorShape: Qt.IBeamCursor
                hoverEnabled: true

                onClicked: (mouse) => {
                    if (root.cppModel) {
                        let clickIndex = codeText.positionAt(mouse.x, mouse.y);
                        root.cppModel.requestFocus(root.block, clickIndex);
                    }
                }
            }
        }

        RowLayout {
            id: buttonRow

            anchors {
                right: parent.right
                top: parent.top
                rightMargin: Kirigami.Units.mediumSpacing
                topMargin: Kirigami.Units.mediumSpacing
            }

            KQuickControlsAddons.Clipboard { id: clipboard }

            Button {
                icon.source: "edit-copy-symbolic"
                onClicked: {
                    clipboard.content = codeText.text
                    showPassiveNotification(i18n("Copied to clipboard!"))
                }
            }
        }
    }
}