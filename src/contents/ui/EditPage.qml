// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.marknote

Kirigami.Page {
    id: root

    property string path
    property string name
    property bool saved : true

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    titleDelegate: RowLayout {
        visible: name
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        Rectangle {
            height:5
            width: 5
            radius: 2.5
            color: Kirigami.Theme.textColor
            visible: !saved
        }
        Kirigami.Heading {
            text: name
            type: saved? Kirigami.Heading.Type.Normal:Kirigami.Heading.Type.Primary

        }
        Item { Layout.fillWidth: true }
    }

    RowLayout {
        id: toolBarContainer
        visible: name
        z: 600000
        parent: root.overlay

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        ToolBar {
            id: toolbar

            Layout.margins: Kirigami.Units.largeSpacing
            Layout.alignment:Qt.AlignHCenter

            background: Kirigami.ShadowedRectangle {
                color: Kirigami.Theme.backgroundColor
                radius: 5

                shadow {
                    size: 15
                    yOffset: 3
                    color: Qt.rgba(0, 0, 0, 0.2)
                }

                border {
                    color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                    width: 1
                }

                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Window
            }
            RowLayout {
                ToolButton {
                    id: boldButton
                    Shortcut {
                        sequence: "Ctrl+B"
                        onActivated: boldButton.clicked()
                    }
                    icon.name: "format-text-bold"
                    text: i18nc("@action:button", "Bold")
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: document.bold
                    onClicked: {
                        document.bold = !document.bold
                    }
                }
                ToolButton {
                    id: italicButton
                    Shortcut {
                        sequence: "Ctrl+I"
                        onActivated: italicButton.clicked()
                    }
                    icon.name: "format-text-italic"
                    text: i18nc("@action:button", "Italic")
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: document.italic
                    onClicked: {
                        document.italic = !document.italic
                    }
                }
                ToolButton {
                    id: underlineButton
                    Shortcut {
                        sequence: "Ctrl+U"
                        onActivated: underlineButton.clicked()
                    }
                    icon.name: "format-text-underline"
                    text: i18nc("@action:button", "Underline")
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: document.underline
                    onClicked: {
                        document.underline = !document.underline
                    }
                }
                ToolButton {
                    enabled: false
                    icon.name: "format-text-strikethrough"
                    text: i18nc("@action:button", "Strikethrough")
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                ToolButton {
                    enabled: false
                    icon.name: "draw-highlight"
                    text: i18nc("@action:button", "highlight")
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                ToolButton {
                    enabled: false
                    icon.name: "format-list-unordered"
                    text: i18n("Add list")
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                ToolButton {
                    enabled: false
                    icon.name: "format-list-ordered"
                    text: i18n("Add numbered list")
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                ComboBox {
                    enabled: false
                    displayText: i18n("Heading %1", parseInt(currentText) + 1)
                    model: 6
                }
            }
        }
    }

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    contentItem: ScrollView {
        TextArea {
            id: textArea

            textMargin: Math.max(Kirigami.Units.gridUnit * 2, toolBarContainer.height)
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            background: null

            onTextChanged: {
                saved = false
                saveTimer.restart()
            }
            persistentSelection: true
            height: parent.height
            textFormat: TextEdit.MarkdownText
            wrapMode: TextEdit.Wrap

            DocumentHandler {
                id: document

                document: textArea.textDocument
                cursorPosition: textArea.cursorPosition
                selectionStart: textArea.selectionStart
                selectionEnd: textArea.selectionEnd
                // textColor: TODO
                onLoaded: (text) => {
                    textArea.text = text
                }
                onError: (message) => {
                    print(message)
                }

                Component.onCompleted: document.load(path)
                Component.onDestruction: document.saveAs(path)
            }

            Timer {
                id: saveTimer

                repeat: false
                interval: 1000
                onTriggered: if (root.name) {
                    document.saveAs(path);
                    saved = true;
                }
            }
        }
    }
}
