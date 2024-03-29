// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
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

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
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
                        document.italic = checked;
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
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
                        document.underline = checked;
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
                ToolButton {
                    icon.name: "format-text-strikethrough"
                    text: i18nc("@action:button", "Strikethrough")
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: document.strikethrough
                    onClicked: {
                        document.strikethrough = checked;
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                ToolButton {
                    id: indentAction
                    icon.name: "format-indent-more"
                    text: i18nc("@action:button", "Increase List Level")
                    display: AbstractButton.IconOnly
                    onClicked: {
                        document.indentListMore();
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                ToolButton {
                    id: dedentAction
                    icon.name: "format-indent-less"
                    text: i18nc("@action:button", "Decrease List Level")
                    display: AbstractButton.IconOnly
                    onClicked: {
                        document.indentListLess();
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                ComboBox {
                    id: listStyleComboBox
                    onActivated: (index) => {
                        document.setListStyle(index);
                    }
                    model: [
                        i18nc("@item:inmenu no list style", "No list"),
                        i18nc("@item:inmenu disc list style", "Disc"),
                        i18nc("@item:inmenu circle list style", "Circle"),
                        i18nc("@item:inmenu square list style", "Square"),
                        i18nc("@item:inmenu numbered lists", "123"),
                        i18nc("@item:inmenu lowercase abc lists", "abc"),
                        i18nc("@item:inmenu uppercase abc lists", "ABC"),
                        i18nc("@item:inmenu lower case roman numerals", "i ii iii"),
                        i18nc("@item:inmenu upper case roman numerals", "I II III")
                    ]
                }

                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }

                ComboBox {
                    id: headingLevelComboBox

                    model: [
                        i18nc("@item:inmenu no heading", "Basic text"),
                        i18nc("@item:inmenu heading level 1 (largest)", "Title"),
                        i18nc("@item:inmenu heading level 2", "Subtitle"),
                        i18nc("@item:inmenu heading level 3", "Section"),
                        i18nc("@item:inmenu heading level 4", "Subsection"),
                        i18nc("@item:inmenu heading level 5", "Paragraph"),
                        i18nc("@item:inmenu heading level 6 (smallest)", "Subparagraph")
                    ]

                    onActivated: (index) => {
                        document.setHeadingLevel(index);
                    }
                }
            }
        }
    }

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    ScrollView {
        anchors.fill: parent

        T.TextArea {
            id: textArea

            textMargin: Math.max(Kirigami.Units.gridUnit * 2, toolBarContainer.height)
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                                    implicitBackgroundWidth + leftInset + rightInset)
            implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                                     implicitBackgroundHeight + topInset + bottomInset)

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: background == null

            color: Kirigami.Theme.textColor
            selectionColor: Kirigami.Theme.highlightColor
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            placeholderTextColor: Kirigami.Theme.disabledTextColor

            selectByMouse: true
            background: null

            onPressAndHold: {
                if (Kirigami.Settings.tabletMode && selectByMouse) {
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }

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

                Component.onCompleted: document.load(root.path)
                Component.onDestruction: document.saveAs(root.path)

                onCursorPositionChanged: {
                    indentAction.enabled = document.canIndentList;
                    dedentAction.enabled = document.canDedentList;
                    listStyleComboBox.currentIndex = document.currentListStyle;
                    headingLevelComboBox.currentIndex = document.currentHeadingLevel
                }
            }

            Timer {
                id: saveTimer

                repeat: false
                interval: 1000
                onTriggered: if (root.name) {
                    document.saveAs(root.path);
                    saved = true;
                }
            }
        }
    }
}
