// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts

import "components"


import org.kde.marknote

Kirigami.Page {
    id: root

    objectName: "EditPage"

    property bool wideScreen: applicationWindow().width >= toolbar.width + Kirigami.Units.largeSpacing * 2

    property bool saved: true
    property string oldPath: ''
    property bool listIndent: true
    property bool listDedent: true
    property bool checkbox: false
    property int listStyle
    property int heading

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    titleDelegate: RowLayout {
        visible: NavigationController.noteName
        Layout.fillWidth: true
        Item {
            width: fillWindowButton.width
            visible: wideScreen
        }
        Item { Layout.fillWidth: true }
        Rectangle {
            height:5
            width: 5
            radius: 2.5
            color: Kirigami.Theme.textColor
            visible: !root.saved
        }
        Kirigami.Heading {
            text: NavigationController.noteName
            type: root.saved? Kirigami.Heading.Type.Normal:Kirigami.Heading.Type.Primary

        }
        Item { Layout.fillWidth: true }
        ToolButton {
            id: fillWindowButton
            visible: wideScreen
            icon.name: "view-fullscreen"
            text: i18n("Focus Mode")
            display: AbstractButton.IconOnly
            checkable: true
            checked: Config.fillWindow
            onClicked: Config.fillWindow = !Config.fillWindow

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    LinkDialog {
        id: linkDialog

        parent: applicationWindow().overlay
        onAccepted: document.updateLink(linkUrl, linkText)
    }

    ImageDialog {
        id: imageDialog

        parent: applicationWindow().overlay
        onAccepted: document.insertImage(imagePath)
        notePath: NavigationController.noteFullPath
    }

    TableDialog {
        id: tableDialog
        parent: applicationWindow().overlay
        onAccepted: document.insertTable(rows, cols)
    }
    Component {
        id: textFormatGroup
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
    }
    }
    Component {
        id: listFormatGroup
        RowLayout {
        ToolButton {
            id: indentAction
            icon.name: "format-indent-more"
            text: i18nc("@action:button", "Increase List Level")
            display: AbstractButton.IconOnly
            onClicked: {
                document.indentListMore();

            }
            enabled: root.listIndent
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
            enabled: root.listDedent
            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
    }
    Component{
    id: listStyleGroup
        ComboBox {
            id: listStyleComboBox
            onActivated: (index) => {
                document.setListStyle(currentValue);
            }
            currentIndex: root.listStyle
            enabled: indentAction.enabled || dedentAction.enabled
            textRole: "text"
            valueRole: "value"
            model: [
                { text: i18nc("@item:inmenu no list style", "No list"), value: 0 },
                { text: i18nc("@item:inmenu unordered style", "Unordered list"), value: 1 },
                { text: i18nc("@item:inmenu ordered style", "Ordered list"), value: 4 },
            ]
        }
    }
    Component{
        id: insertGroup
        RowLayout {
        ToolButton {
            id: checkboxAction
            icon.name: "checkbox-symbolic"
            text: i18nc("@action:button", "Insert checkbox")
            display: AbstractButton.IconOnly
            checkable: true
            onClicked: {
                document.checkable = !document.checkable;
            }
            checked: root.checkbox
            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            id: linkAction
            icon.name: "insert-link-symbolic"
            text: i18nc("@action:button", "Insert link")
            display: AbstractButton.IconOnly
            onClicked: {
                linkDialog.linkText = document.currentLinkText();
                linkDialog.linkUrl = document.currentLinkUrl();
                linkDialog.open();
            }

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            id: imageAction
            icon.name: "insert-image-symbolic"
            text: i18nc("@action:button", "Insert image")
            display: AbstractButton.IconOnly
            onClicked: {
                imageDialog.open();
            }

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        ToolButton {
            id: tableAction
            icon.name: "insert-table"
            text: i18nc("@action:button", "Insert table")
            display: AbstractButton.IconOnly
            onClicked: {
                tableDialog.open()
            }

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

    }
    }
    Component{
        id: headingGroup
        ComboBox {
            id: headingLevelComboBox
            currentIndex: root.heading

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

    RowLayout {
        id: mobileToolBarContainer
        visible: !wideScreen

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        z: 600000
        parent: root.overlay
        MouseArea {
            anchors.fill: parent
        }
        Kirigami.ShadowedRectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.gridUnit *5

            shadow {
                size: 15
                color: Qt.rgba(0, 0, 0, 0.2)
            }
            Kirigami.Separator {
                width: parent.width
                anchors.top: parent.top

            }

            ColumnLayout {
                id: mobileToolbarLayout
                anchors.fill: parent
                SwipeView{
                    id: swipeView
                    clip: true
                    Layout.margins: Kirigami.Units.mediumSpacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: categorySelector.selectedIndex
                    interactive: false
                    Item {
                        id: firstPage
                        RowLayout {
                            width: swipeView.width
                            Loader { sourceComponent: textFormatGroup }
                            Item { Layout.fillWidth: true }
                            Loader { sourceComponent: headingGroup }
                        }
                    }
                    Item {
                        id: secondPage
                        RowLayout {
                            width: swipeView.width
                            Loader { sourceComponent: listFormatGroup }
                            Item { Layout.fillWidth: true }
                            Loader { sourceComponent: listStyleGroup }
                        }                    }
                    Item {
                        id: thirdPage
                        Loader { sourceComponent: insertGroup }
                    }

                }
                RadioSelector{
                    id: categorySelector

                    Layout.rightMargin: Kirigami.Units.mediumSpacing
                    Layout.leftMargin: Kirigami.Units.mediumSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                    Layout.fillWidth: true
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                    Layout.alignment: Qt.AlignHCenter

                    consistentWidth: true

                    actions: [
                       Kirigami.Action {
                           text: i18n("Format")
                       },
                       Kirigami.Action {
                           text: i18n("Lists")
                       },
                       Kirigami.Action {
                            text: i18n("Insert")
                        }
                   ]
                }

            }
        }
    }
    RowLayout {
        id: toolBarContainer
        visible: wideScreen


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

            background: ToolbarBackground {}
            RowLayout {
                Loader { sourceComponent: textFormatGroup }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                Loader { sourceComponent: listFormatGroup }
                Loader { sourceComponent: listStyleGroup }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                Loader { sourceComponent: insertGroup }
                Kirigami.Separator {
                    Layout.fillHeight: true
                    Layout.margins: 0
                }
                Loader { sourceComponent: headingGroup }
            }
        }
    }

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    contentItem: ScrollView {
        T.TextArea {
            id: textArea

            textMargin: Kirigami.Units.gridUnit * 2
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: mobileToolBarContainer.height

            font: Config.editorFont

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

            property int lastKey
            Keys.onPressed: (event) => lastKey = event.key

            onTextChanged: {
                document.slotKeyPressed(lastKey)
                root.saved = false;
                saveTimer.restart()
            }
            persistentSelection: true
            height: parent.height
            textFormat: TextEdit.MarkdownText
            wrapMode: TextEdit.Wrap

            Connections {
                target: NavigationController

                function onNotePathChanged(): void {
                    if (oldPath.length > 0 && !saved) {
                        document.saveAs(oldPath);
                    }
                    if (NavigationController.notePath.length > 0) {
                        document.load(NavigationController.noteFullPath);
                        root.saved = true;
                    }
                    oldPath = NavigationController.noteFullPath;

                    textArea.forceActiveFocus();
                }
            }

            DocumentHandler {
                id: document

                document: textArea.textDocument
                textArea: textArea
                cursorPosition: textArea.cursorPosition
                selectionStart: textArea.selectionStart
                selectionEnd: textArea.selectionEnd
                // textColor: TODO
                onLoaded: (text) => {
                    textArea.text = text
                }
                onError: (message) => {
                    console.error("Error message from document handler", message)
                }

                onCopy: textArea.copy();
                onCut: textArea.cut();
                onUndo: textArea.undo();
                onRedo: textArea.redo();

                Component.onCompleted: {
                    if (NavigationController.notePath.length > 0) {
                        document.load(NavigationController.noteFullPath);
                        root.saved = true;
                        oldPath = NavigationController.noteFullPath;
                        textArea.forceActiveFocus();
                    }
                }

                Component.onDestruction: {
                    if (!saved && NavigationController.notePath.length > 0) {
                        document.saveAs(NavigationController.noteFullPath);
                    }
                }

                onCheckableChanged: {
                    root.checkbox = document.checkable;
                }

                onMoveCursor: (position) => {
                    textArea.cursorPosition = position;
                }

                onCursorPositionChanged: {
                    root.listIndent = document.canIndentList;
                    root.listDedent = document.canDedentList;
                    root.checkbox = document.checkable;

                    if (document.currentListStyle === 0) {
                        root.listStyle = 0;
                    } else if (document.currentListStyle === 1) {
                        root.listStyle = 1;
                    } else if (document.currentListStyle === 4) {
                        root.listStyle = 2;
                    }
                    root.heading = document.currentHeadingLevel
                }
            }

            Timer {
                id: saveTimer

                repeat: false
                interval: 1000
                onTriggered: if (NavigationController.notePath.length > 0) {
                    document.saveAs(NavigationController.noteFullPath);
                    saved = true;
                }
            }
        }
    }
}
