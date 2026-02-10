// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
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

    property bool checkbox: false
    property alias document: document
    property int heading
    property bool init: false
    property bool listDedent: true
    property bool listIndent: true
    property int listStyle
    property string noteFullPath: NavigationController.noteFullPath

    // Only overwrite these values in MainEditor
    property string noteName: NavigationController.noteName
    property string oldPath: ''
    property bool saved: true
    property bool singleDocumentMode: false
    readonly property bool wideScreen: (width >= toolBar.width + Kirigami.Units.largeSpacing * 2) && pageStack.columnView.columnResizeMode !== Kirigami.ColumnView.SingleColumn

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
    objectName: "RichEditPage"
    rightPadding: 0
    topPadding: 0

    contentItem: ScrollView {
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        T.TextArea {
            id: textArea

            // To eliminate text overlap by the textFormatGroup we introduce extra padding
            readonly property int additionalPadding: Kirigami.Units.gridUnit * 4
            property int lastKey: -1

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: background == null
            background: null
            bottomPadding: wideScreen ? additionalPadding : (mobileToolBarContainer.hidden ? 0 : mobileToolBarContainer.height)
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
            textFormat: TextEdit.MarkdownText
            textMargin: wideScreen ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 1
            topPadding: 0
            wrapMode: TextEdit.Wrap

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            Keys.onPressed: event => {
                lastKey = event.key;
            }
            onPressAndHold: {
                if (Kirigami.Settings.tabletMode && selectByMouse) {
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }
            onTextChanged: {
                if (lastKey !== -1) {
                    let key = lastKey;
                    lastKey = -1;
                    document.slotKeyPressed(key);
                }
                root.saved = false;
                saveTimer.restart();
            }

            TableActionHelper {
                id: tableHelper

                cursorPosition: textArea.cursorPosition
                document: textArea.textDocument
                selectionEnd: textArea.selectionEnd
                selectionStart: textArea.selectionStart
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

                RichDocumentHandler {
                    id: document

                    cursorPosition: textArea.cursorPosition
                    document: textArea.textDocument
                    selectionEnd: textArea.selectionEnd
                    selectionStart: textArea.selectionStart
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
                    onCheckableChanged: {
                        root.checkbox = document.checkable;
                    }
                    onCopy: textArea.copy()
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
                        root.heading = document.currentHeadingLevel;
                    }
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
            // for spacing
            width: Kirigami.Units.largeSpacing*5
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn
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

            Button{
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: i18n("Switch editor to source mode")
                ToolTip.visible: hovered
                icon.name: "code-context-symbolic"
                checkable: true
                checked: false
                text: i18n("Source View")
                padding: 0
                flat: true
                spacing: Kirigami.Units.mediumSpacing

                onClicked: {
                    NavigationController.sourceMode = !NavigationController.sourceMode
                }
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

    LinkDialog {
        id: linkDialog

        parent: applicationWindow().overlay

        onAccepted: document.updateLink(linkUrl, linkText)
    }
    ImageDialog {
        id: imageDialog

        notePath: root.noteFullPath
        parent: applicationWindow().overlay

        onAccepted: {
            if (imagePath.toString().length > 0) {
                document.insertImage(imagePath);
                imagePath = '';
            }
        }
    }
    TableDialog {
        id: tableDialog

        parent: applicationWindow().overlay

        onAccepted: document.insertTable(rows, cols)
    }
    Component {
        id: textFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                id: boldButton

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: document.bold
                display: AbstractButton.IconOnly
                icon.name: "format-text-bold"
                text: i18nc("@action:button", "Bold")

                onClicked: {
                    document.bold = !document.bold;
                }

                Shortcut {
                    sequence: "Ctrl+B"

                    onActivated: boldButton.clicked()
                }
            }
            ToolButton {
                id: italicButton

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: document.italic
                display: AbstractButton.IconOnly
                icon.name: "format-text-italic"
                text: i18nc("@action:button", "Italic")

                onClicked: {
                    document.italic = checked;
                }

                Shortcut {
                    sequence: "Ctrl+I"

                    onActivated: italicButton.clicked()
                }
            }
            ToolButton {
                id: underlineButton

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: document.underline
                display: AbstractButton.IconOnly
                icon.name: "format-text-underline"
                text: i18nc("@action:button", "Underline")

                onClicked: {
                    document.underline = checked;
                }

                Shortcut {
                    sequence: "Ctrl+U"

                    onActivated: underlineButton.clicked()
                }
            }
            ToolButton {
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: document.strikethrough
                display: AbstractButton.IconOnly
                icon.name: "format-text-strikethrough"
                text: i18nc("@action:button", "Strikethrough")

                onClicked: {
                    document.strikethrough = checked;
                }
            }
        }
    }
    Kirigami.Action {
        id: indentAction

        enabled: root.listIndent
        icon.name: "format-indent-more"
        text: i18nc("@action:button", "Increase List Level")

        onTriggered: {
            document.indentListMore();
        }
    }
    Kirigami.Action {
        id: dedentAction

        enabled: root.listDedent
        icon.name: "format-indent-less"
        text: i18nc("@action:button", "Decrease List Level")

        onTriggered: {
            document.indentListLess();
        }
    }
    Component {
        id: listFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                action: indentAction
                display: AbstractButton.IconOnly
            }
            ToolButton {
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                action: dedentAction
                display: AbstractButton.IconOnly
            }
        }
    }
    Component {
        id: listStyleGroup

        ComboBox {
            id: listStyleComboBox

            currentIndex: root.listStyle
            enabled: indentAction.enabled || dedentAction.enabled
            model: [
                {
                    text: i18nc("@item:inmenu no list style", "No list"),
                    value: 0
                },
                {
                    text: i18nc("@item:inmenu unordered style", "Unordered list"),
                    value: 1
                },
                {
                    text: i18nc("@item:inmenu ordered style", "Ordered list"),
                    value: 4
                },
            ]
            textRole: "text"
            valueRole: "value"

            onActivated: index => {
                document.setListStyle(currentValue);
            }
        }
    }
    Component {
        id: insertGroup

        RowLayout {
            ToolButton {
                id: checkboxAction

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: root.checkbox
                display: AbstractButton.IconOnly
                icon.name: "checkbox-symbolic"
                text: i18nc("@action:button", "Insert checkbox")

                onClicked: {
                    document.checkable = !document.checkable;
                }
            }
            ToolButton {
                id: linkAction

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                display: AbstractButton.IconOnly
                icon.name: "insert-link-symbolic"
                text: i18nc("@action:button", "Insert link")

                onClicked: {
                    linkDialog.linkText = document.currentLinkText();
                    linkDialog.linkUrl = document.currentLinkUrl();
                    linkDialog.open();
                }
            }
            ToolButton {
                id: imageAction

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                display: AbstractButton.IconOnly
                icon.name: "insert-image-symbolic"
                text: i18nc("@action:button", "Insert image")

                onClicked: {
                    imageDialog.open();
                }
            }
            ToolButton {
                id: tableAction

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                display: AbstractButton.IconOnly
                icon.name: "insert-table"
                text: i18nc("@action:button", "Insert table")

                onClicked: {
                    tableDialog.open();
                }
            }
        }
    }
    Component {
        id: headingGroup

        ComboBox {
            id: headingLevelComboBox

            currentIndex: root.heading
            model: [i18nc("@item:inmenu no heading", "Basic text"), i18nc("@item:inmenu heading level 1 (largest)", "Title"), i18nc("@item:inmenu heading level 2", "Subtitle"), i18nc("@item:inmenu heading level 3", "Section"), i18nc("@item:inmenu heading level 4", "Subsection"), i18nc("@item:inmenu heading level 5", "Paragraph"), i18nc("@item:inmenu heading level 6 (smallest)", "Subparagraph")]

            onActivated: index => {
                document.setHeadingLevel(index);
            }
        }
    }
    Components.FloatingButton {
        icon.name: "document-edit"
        parent: root.overlay
        scale: mobileToolBarContainer.hidden ? 1 : 0
        visible: !wideScreen

        Behavior on scale {
            NumberAnimation {
                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        onClicked: mobileToolBarContainer.hidden = false

        anchors {
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit
        }
    }
    RowLayout {
        id: mobileToolBarContainer

        property bool hidden: false

        parent: root.overlay
        visible: !wideScreen
        y: hidden ? parent.height : parent.height - mobileToolBar.height
        z: 600000

        Behavior on y {
            NumberAnimation {
                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        anchors {
            left: parent.left
            right: parent.right
        }
        Kirigami.ShadowedRectangle {
            id: mobileToolBar

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.gridUnit * 5 + Kirigami.Units.smallSpacing * 2

            shadow {
                color: Qt.rgba(0, 0, 0, 0.2)
                size: 15
            }
            MouseArea {
                anchors.fill: parent
            }
            Kirigami.Separator {
                anchors.top: parent.top
                width: parent.width
            }
            ColumnLayout {
                id: mobileToolbarLayout

                anchors.fill: parent

                RowLayout {
                    SwipeView {
                        id: swipeView

                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.mediumSpacing
                        clip: true
                        currentIndex: categorySelector.selectedIndex
                        implicitHeight: undoButton.height + Kirigami.Units.smallSpacing
                        interactive: false

                        Item {
                            id: firstPage

                            RowLayout {
                                height: swipeView.height
                                width: swipeView.width

                                Loader {
                                    sourceComponent: textFormatGroup
                                }
                                Item {
                                    Layout.fillWidth: true
                                }
                                Loader {
                                    sourceComponent: headingGroup
                                }
                            }
                        }
                        Item {
                            id: secondPage

                            RowLayout {
                                height: swipeView.height
                                width: swipeView.width

                                Loader {
                                    sourceComponent: listFormatGroup
                                }
                                Item {
                                    Layout.fillWidth: true
                                }
                                Loader {
                                    sourceComponent: listStyleGroup
                                }
                            }
                        }
                        Item {
                            id: thirdPage

                            RowLayout {
                                height: swipeView.height
                                width: swipeView.width

                                Loader {
                                    sourceComponent: insertGroup
                                }
                            }
                        }
                    }
                    Kirigami.Separator {
                        Layout.bottomMargin: Kirigami.Units.mediumSpacing
                        Layout.fillHeight: true
                        Layout.topMargin: Kirigami.Units.mediumSpacing
                    }
                    ToolButton {
                        id: undoButton
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        //                        Layout.topMargin: Kirigami.Units.smallSpacing
                        //                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        ToolTip.text: text
                        ToolTip.visible: hovered
                        display: AbstractButton.IconOnly
                        enabled: textArea.canUndo
                        icon.name: "edit-undo"
                        text: i18n("Undo")

                        onClicked: textArea.undo()
                    }
                    ToolButton {

                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        //                        Layout.topMargin: Kirigami.Units.smallSpacing
                        //                        Layout.bottomMargin: Kirigami.Units.smallSpacing

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        display: AbstractButton.IconOnly
                        enabled: textArea.canRedo
                        icon.name: "edit-redo"
                        text: i18n("Redo")

                        onClicked: textArea.redo()
                    }
                }
                RowLayout {
                    Layout.fillWidth: true

                    Item {
                        Layout.fillWidth: true
                    }
                    Components.RadioSelector {
                        id: categorySelector

                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                        Layout.topMargin: 0
                        consistentWidth: true

                        actions: [
                            Kirigami.Action {
                                text: i18n("Format")
                                //                           icon.name: "format-border-style"
                            },
                            Kirigami.Action {
                                text: i18n("Lists")
                                //                           icon.name: "media-playlist-append"
                            },
                            Kirigami.Action {
                                text: i18n("Insert")
                                //                           icon.name: "kdenlive-add-text-clip"
                            }
                        ]
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignRight
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.rightMargin: Kirigami.Units.mediumSpacing
                        Layout.topMargin: 0
                        height: categorySelector.height
                        icon.height: Kirigami.Units.gridUnit
                        icon.name: "arrow-down"
                        icon.width: Kirigami.Units.gridUnit
                        width: height

                        onClicked: mobileToolBarContainer.hidden = true
                    }
                }
            }
        }
    }
    Components.FloatingToolBar {
        id: toolBar

        parent: root.overlay
        visible: wideScreen
        z: 600000

        contentItem: RowLayout {
            Loader {
                sourceComponent: textFormatGroup
            }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader {
                sourceComponent: listFormatGroup
            }
            Loader {
                sourceComponent: listStyleGroup
            }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader {
                sourceComponent: insertGroup
            }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader {
                sourceComponent: headingGroup
            }
        }

        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: Kirigami.Units.largeSpacing
        }
    }
    TextFieldContextMenu {
        id: textFieldContextMenu

        tableActionHelper: tableHelper
    }
}
