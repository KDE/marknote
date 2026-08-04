// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.marknote

Item {
    id: root

    implicitHeight: editing ? textEdit.implicitHeight : textView.implicitHeight
    implicitWidth: editing ? textEdit.implicitWidth : textView.implicitWidth

    required property var block
    required property var html
    required property var md
    required property var blockType
    required property var delegateModel
    required property var index

    onMdChanged: {
        if (textEdit) {
            textEdit.lastSavedText = root.md;
        }
    }

    readonly property var model: delegateModel.model
    readonly property var modelIndex: delegateModel.modelIndex(index)

    property var color: Kirigami.Theme.textColor
    property int padding: 0
    property int fontSize: Kirigami.Theme.defaultFont.pointSize
    property int fontBold: Kirigami.Theme.defaultFont.bold
    property string fontFamily: Kirigami.Theme.defaultFont.family

    property bool editing: false
    property int wrapMode: TextEdit.Wrap

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

            model.requestFocus(root.block, cursorPosition);
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

        placeholderText: {
            let currentType = root.block.data.type;
            let parentType = root.block.parent ? root.block.parent.data.type : -1;

            if (currentType === MDOptions.ElementType.Heading) {
                return "Heading";
            } else if (parentType === MDOptions.ElementType.ListItem) {
                return "List item";
            } else if (parentType === MDOptions.ElementType.Blockquote) {
                return "Quote";
            } else {
                return "Type here...";
            }
        }

        property string lastSavedText: root.md
        property int lastSavedCursorPos: 0
        property bool lastSavedTextParsed: true

        Timer {
            id: editTimer
            interval: 500
            repeat: false
            onTriggered: {
                if (textEdit.lastSavedText !== textEdit.text) {
                    CommandManager.editText(root.block, textEdit.lastSavedText, textEdit.text, textEdit.lastSavedCursorPos, textEdit.cursorPosition);
                    textEdit.lastSavedText = textEdit.text;
                    textEdit.lastSavedCursorPos = textEdit.cursorPosition;
                    textEdit.lastSavedTextParsed = false
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
            if (root.editing && textEdit.text !== textEdit.lastSavedText) {
                editTimer.restart();
            }
        }

        Keys.onReturnPressed: event => {
            if (event.modifiers & Qt.ShiftModifier) {
                event.accepted = false;
                return;
            }

            flushTimer();
            const cursorPos = textEdit.cursorPosition;

            if (root.block.parent.data.type === MDOptions.ElementType.ListItem) {
                if (textEdit.text == "") {
                    if (!CommandManager.deIndentListItem(root.block, cursorPos)) {
                        CommandManager.convertToParagraph(root.block, cursorPos);
                    }
                } else {
                    textEdit.lastSavedTextParsed = true;
                    CommandManager.splitListItem(root.block, textEdit.text, cursorPos);
                }
            } else if (root.block.parent.data.type === MDOptions.ElementType.Blockquote) {
                if (!textEdit.text == "" || !CommandManager.moveOutsideBlockquote(root.block, cursorPos)) {
                    textEdit.lastSavedTextParsed = true;
                    CommandManager.splitBlock(root.block, textEdit.text, cursorPos);
                }
            } else {
                textEdit.lastSavedTextParsed = true;
                CommandManager.splitBlock(root.block, textEdit.text, textEdit.cursorPosition);
            }

            root.editing = false;
            event.accepted = true
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Space) {
                if (CommandManager.autoTransform(root.block, textEdit.text, textEdit.cursorPosition, index)) {
                    textEdit.text = root.md;
                    event.accepted = true;
                    flushTimer();
                    return;
                }
            }

            if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                flushTimer();
                textEdit.lastSavedTextParsed = true;
                if (event.modifiers & Qt.ShiftModifier) {
                    CommandManager.redo();
                } else {
                    CommandManager.undo();
                }
                event.accepted = true;
            }

            if (event.key === Qt.Key_Backspace && textEdit.cursorPosition === 0) {
                flushTimer();
                if (CommandManager.convertToParagraph(root.block, 0)) {
                    event.accepted = true;
                    return;
                }

                CommandManager.mergeWithPreviousBlock(root.block, textEdit.text);
            }

            if (event.key === Qt.Key_Backtab) {
                if (root.block.parent.data.type === MDOptions.ElementType.ListItem) {
                    flushTimer();
                    CommandManager.deIndentListItem(root.block, textEdit.cursorPosition)
                    event.accepted = true
                }
            }

            if (event.key === Qt.Key_Tab) {
                if (root.block.parent.data.type === MDOptions.ElementType.ListItem) {
                    flushTimer();
                    CommandManager.indentListItem(root.block, textEdit.cursorPosition)
                    event.accepted = true
                }
            }

            if (event.key === Qt.Key_Up) {
                let textBeforeCursor = textEdit.text.substring(0, textEdit.cursorPosition);
                if (textBeforeCursor.indexOf('\n') === -1) {
                    CommandManager.moveToPreviousBlock(root.block, textEdit.text, textEdit.cursorPosition);
                    flushTimer();
                    event.accepted = true;
                }
            }

            if (event.key === Qt.Key_Down) {
                let textAfterCursor = textEdit.text.substring(textEdit.cursorPosition);
                if (textAfterCursor.indexOf('\n') === -1) {
                    CommandManager.moveToNextBlock(root.block, textEdit.text, textEdit.cursorPosition);
                    flushTimer();
                    event.accepted = true;
                }
            }
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                if (model.focusedBlock() === root.block) {
                    // Focus lost due to some unknown reason
                    // We need to bring it back
                    Qt.callLater(() => {
                        model.requestFocus(root.block, model.focusedBlockCursorPos());
                    })
                    return;
                }

                root.editing = false;
                
                flushTimer();
                
                if (!lastSavedTextParsed) {
                    CommandManager.parseBlock(root.block, textEdit.text);
                    lastSavedTextParsed = true;
                }
            } else {
                textEdit.lastSavedCursorPos = textEdit.cursorPosition;
                model.setFocusedBlock(root.block, textEdit.cursorPosition);
            }
        }
    }

    Connections {
        target: model

        function onFocusRequested(requestedBlock, cursorPosition) {
            if (requestedBlock === root.block) {
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
            if (root.block === model.focusedBlock()) {
                root.editing = true;

                textEdit.cursorPosition = model.focusedBlockCursorPos();

                textEdit.forceActiveFocus();
            }
        })
    }
}
