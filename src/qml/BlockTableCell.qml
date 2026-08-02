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

            // Custom focusing mechanism will be implemented here
            root.editing = true;
            textEdit.forceActiveFocus();
            textEdit.cursorPosition = cursorPosition;
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

        onActiveFocusChanged: {
            if (!activeFocus) {
                root.editing = false;
            }
        }
    }
}
