
import QtQuick 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

import org.kde.marknote 1.0

Kirigami.ScrollablePage {
    id: root
    property string path
    property string name
    property bool saved : true
    Kirigami.Theme.colorSet: Kirigami.Theme.View
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
        visible: name
        z: 600000
        y: root.height - 100
        width: root.width
        parent: root.overlay
        ToolBar {
            id: toolbar

            Layout.margins: 10
            Layout.alignment:Qt.AlignHCenter
            background: Kirigami.ShadowedRectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Window
                shadow.size: 15
                shadow.yOffset: 3
                shadow.color: Qt.rgba(0, 0, 0, 0.2)
                color: Kirigami.Theme.backgroundColor
                border.color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                border.width: 1
                radius: 5
            }
            RowLayout {
                ToolButton {
                    id: boldButton
                    Shortcut {
                        sequence: "Ctrl+B"
                        onActivated: boldButton.clicked()
                    }
                    icon.name: "format-text-bold"
                    text: "Bold"
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
                    text: "Italic"
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: document.italic
                    onClicked: {
                        document.italic = !document.italic
                    }
                }
                ToolButton {
                    enabled: false
                    icon.name: "format-text-strikethrough"
                    text: "Strikethrough"
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                ToolButton {
                    enabled: false
                    icon.name: "draw-highlight"
                    text: "highlight"
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
                    text: "list"
                    display: AbstractButton.IconOnly
                    checkable: true

                }
                ToolButton {
                    enabled: false
                    icon.name: "format-list-ordered"
                    text: "numbered list"
                    display: AbstractButton.IconOnly
                    checkable: true
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                ComboBox {
                    enabled: false
                    model:[ "Heading 1","Heading 2","Heading 3","Heading 4","Heading 5","Heading 6" ]

                }

            }
        }
    }
    RowLayout{
        visible: name
        width: root.width
        height: flickable.contentHeight
        Flickable {

            id: flickable
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: Kirigami.Units.gridUnit * 40
            Layout.margins: 0
            Layout.fillHeight: true
            Layout.fillWidth: true
            contentWidth: width

            TextArea.flickable: TextArea {
                id: textArea
                background: Item {

                }
                onTextChanged: {
                    saved = false
                    saveTimer.restart()
                }
                persistentSelection: true
                textMargin: Kirigami.Units.gridUnit
                height: parent.height
                textFormat: TextEdit.MarkdownText
                wrapMode: TextEdit.WordWrap

                DocumentHandler {
                    id: document
                    document: textArea.textDocument
                    cursorPosition: textArea.cursorPosition
                    selectionStart: textArea.selectionStart
                    selectionEnd: textArea.selectionEnd
                    // textColor: TODO
                    Component.onCompleted: document.load(path)
                    Component.onDestruction: document.saveAs(path)
                    onLoaded: {
                        textArea.text = text
                    }
                    onError: (message) => {
                        print(message)
                    }
                }
            }

            Timer{
                id: saveTimer
                repeat: false
                interval: 1000
                onTriggered: {
                    if (root.name) {
                        document.saveAs(path)
                        console.log("timer ")
                        saved = true
                    }
                }
            }
        }
    }
}
