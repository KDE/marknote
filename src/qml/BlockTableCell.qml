// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.marknote

Item {
    id: root

    implicitHeight: editing ? textEdit.implicitHeight : textView.implicitHeight
    implicitWidth: editing ? textEdit.implicitWidth : textView.implicitWidth

    required property var block
    required property var html
    required property var md
    required property var model
    required property int rowIndex
    required property int columnIndex

    property var color: Kirigami.Theme.textColor
    property int padding: 0
    property int fontSize: Kirigami.Theme.defaultFont.pointSize
    property int fontBold: Kirigami.Theme.defaultFont.bold
    property string fontFamily: Kirigami.Theme.defaultFont.family

    property bool editing: false
    property int wrapMode: TextEdit.Wrap

    property string lastSavedText: ""
    property int lastSavedCursorPos: 0
    property bool lastSavedTextParsed: false

    function flushTimer() {
        if (editTimer.running) {
            editTimer.stop();
            if (lastSavedText !== textEdit.text) {
                CommandManager.editTableCellText(root.block, root.rowIndex, root.columnIndex, lastSavedText, textEdit.text, lastSavedCursorPos, textEdit.cursorPosition);
            }
            lastSavedText = textEdit.text;
            lastSavedCursorPos = textEdit.cursorPosition;
        }
    }

    Timer {
        id: editTimer

        interval: 500
        onTriggered: {
            if (lastSavedText !== textEdit.text) {
                CommandManager.editTableCellText(root.block, root.rowIndex, root.columnIndex, lastSavedText, textEdit.text, lastSavedCursorPos, textEdit.cursorPosition);
            }

            lastSavedText = textEdit.text;
            lastSavedCursorPos = textEdit.cursorPosition;
        }
    }

    TextEdit {
        id: textView

        anchors.fill: parent
        text: root.html
        textFormat: Text.RichText
        color: root.color
        wrapMode: root.wrapMode
        visible: !root.editing
        readOnly: true
        selectByMouse: false
        padding: root.padding
        font.pointSize: root.fontSize
        font.bold: root.fontBold
        font.family: root.fontFamily
    }

    MouseArea {
        anchors.fill: textView
        enabled: !root.editing
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor

        onClicked: (mouse) => {
            let clickIndex = textView.positionAt(mouse.x, mouse.y);
            const cursorPosition = CommandManager.getCursorInMdString(textView.getText(0, textView.text.length), root.md, clickIndex);

            model.requestFocusOnTable(root.block, root.rowIndex, root.columnIndex, cursorPosition);
        }
    }

    TextArea {
        id: textEdit

        text: root.md
        anchors.fill: parent
        visible: root.editing
        wrapMode: root.wrapMode
        background: null
        padding: root.padding
        font.pointSize: root.fontSize
        font.bold: root.fontBold
        font.family: root.fontFamily
        color: root.color

        property bool isProgrammaticUpdate: false

        onTextChanged: {
            if (isProgrammaticUpdate) return;
            if (textEdit.activeFocus && textEdit.text !== lastSavedText && !lastSavedTextParsed) {
                editTimer.restart();
            }
            lastSavedTextParsed = false;
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
                return;
            }

            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                if (event.modifiers & Qt.ShiftModifier) {
                    return;
                }

                CommandManager.moveToBottomTableCell(root.block, root.rowIndex, root.columnIndex, textEdit.cursorPosition);
                event.accepted = true;
                return;
            }

            if (textEdit.selectedText) {
                return;
            }

            if (event.key === Qt.Key_Left && textEdit.cursorPosition === 0) {
                CommandManager.moveToLeftTableCell(root.block, root.rowIndex, root.columnIndex);
                event.accepted = true;
            } else if (event.key === Qt.Key_Right && textEdit.cursorPosition === textEdit.text.length) {
                CommandManager.moveToRightTableCell(root.block, root.rowIndex, root.columnIndex);
                event.accepted = true;
            } else if (event.key === Qt.Key_Up) {
                let textBeforeCursor = textEdit.text.substring(0, textEdit.cursorPosition);
                if (textBeforeCursor.indexOf('\n') === -1) {
                    CommandManager.moveToTopTableCell(root.block, root.rowIndex, root.columnIndex, textEdit.cursorPosition);
                    event.accepted = true;
                }
            } else if (event.key === Qt.Key_Down) {
                let textAfterCursor = textEdit.text.substring(textEdit.cursorPosition);
                if (textAfterCursor.indexOf('\n') === -1) {
                    CommandManager.moveToBottomTableCell(root.block, root.rowIndex, root.columnIndex, textEdit.cursorPosition);
                    event.accepted = true;
                }
            }
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                if (model.focusedBlock() === root.block && model.focusedTableRow() === root.rowIndex && model.focusedTableColumn() === root.columnIndex) {
                    let activeItem = textEdit.Window.activeFocusItem;
                    let isFallback = !activeItem;
                    let p = root.parent;
                    while (p && !isFallback) {
                        if (activeItem === p) isFallback = true;
                        p = p.parent;
                    }

                    if (!isFallback) {
                        model.setFocusedBlock(null, -1);
                    } else {
                        Qt.callLater(() => {
                            model.requestFocusOnTable(root.block, root.rowIndex, root.columnIndex, model.focusedBlockCursorPos());
                        })
                        return;
                    }
                }

                flushTimer();
                root.editing = false;
            } else {
                lastSavedText = textEdit.text;
                lastSavedCursorPos = textEdit.cursorPosition;
                model.setFocusedBlockToTable(root.block, root.rowIndex, root.columnIndex, textEdit.cursorPosition);
            }
        }
    }

    Connections {
        target: root.model

        function onFocusRequestedOnTable(requestedBlock, row, column, cursorPosition) {
            if (requestedBlock === root.block && row === root.rowIndex && column === root.columnIndex) {
                root.editing = true;

                if (cursorPosition >= 0) {
                    textEdit.cursorPosition = cursorPosition;
                }

                textEdit.forceActiveFocus();
            }
        }
    }

    Component.onCompleted: {
        Qt.callLater(() => {
            if (model.focusedBlock() === root.block && model.focusedTableRow() === root.rowIndex && model.focusedTableColumn() === root.columnIndex) {
                root.editing = true;
                textEdit.cursorPosition = model.focusedBlockCursorPos();
                textEdit.forceActiveFocus();
            }
        })
    }

    Connections {
        target: root

        function onMdChanged() {
            root.lastSavedText = root.md;
            if (textEdit.text !== root.md) {
                textEdit.isProgrammaticUpdate = true;
                textEdit.text = root.md;
                textEdit.isProgrammaticUpdate = false;
            }
        }
    }
}
