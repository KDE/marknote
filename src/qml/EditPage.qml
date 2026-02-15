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

    Layout.fillWidth: true


    objectName: "EditPage"

    readonly property bool wideScreen: width >= toolBar.width + Kirigami.Units.largeSpacing * 2

    property bool saved: true
    property string oldPath: ''
    property bool listIndent: true
    property bool listDedent: true
    property bool checkbox: false
    property int listStyle
    property int heading
    property bool singleDocumentMode: false

    leftPadding: 0
    rightPadding: Kirigami.Settings.isMobile ? 0 : (tocDrawer.width * tocDrawer.position)
    topPadding: 0
    bottomPadding: 0

    // Only overwrite these values in MainEditor
    property string noteName: NavigationController.noteName
    property string noteFullPath: NavigationController.noteFullPath
    property alias document: document

    property bool init: false

    function openSearch(): void {
        if (searchBar) {
            searchBar.isSearchOpen = true;

            if (searchField) {
                // Ensure focus happens slightly after
                // the state change triggers the layout update
                Qt.callLater(function() {
                    searchField.forceActiveFocus();
                    searchField.selectAll();
                });
            }
        }
    }

    function closeSearch(): void {
        if (searchBar) {
            searchBar.isSearchOpen = false;
            textArea.deselect();

            if (textArea) {
                Qt.callLater(function() {
                    textArea.forceActiveFocus();
                });
            }
        }
    }

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

    Component.onCompleted: {
        init = true;
    }
    onVisibleChanged: {
        if (visible) {
            applicationWindow().currentDocument = document
        } else if (applicationWindow().currentDocument === document) {
            applicationWindow().currentDocument = null
        }
    }

    Component.onDestruction: {
        if (applicationWindow().currentDocument === document) {
            applicationWindow().currentDocument = null
        }
    }

    onNoteFullPathChanged: () => {
        if (!init) {
            return;
        }
        loadNote();
    }

    titleDelegate: RowLayout {
        visible: root.noteName
        Layout.fillWidth: true

        ToolButton {
            icon.name: "edit-undo"
            text: i18n("Undo")
            display: AbstractButton.IconOnly
            Layout.leftMargin: Kirigami.Units.smallSpacing
            onClicked: textArea.undo()
            enabled: textArea.canUndo
            visible: !root.singleDocumentMode && !mobileToolbarLayout.visible

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        ToolButton {
            icon.name: "edit-redo"
            text: i18n("Redo")
            display: AbstractButton.IconOnly
            onClicked: textArea.redo()
            enabled: textArea.canRedo
            visible: !root.singleDocumentMode && !mobileToolbarLayout.visible

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }



        Item { Layout.fillWidth: true }
        Rectangle {
            height: 5
            width: height
            radius: 2.5
            scale: root.saved ? 0 : 1
            color: Kirigami.Theme.textColor
            Behavior on scale {
                NumberAnimation {

                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

        }
        Kirigami.Heading {
            text: root.noteName
            Layout.rightMargin: Kirigami.Units.mediumSpacing
            Layout.leftMargin: Kirigami.Units.mediumSpacing

        }
        Item{ width: 5 }

        Item { Layout.fillWidth: true }
        Item {
            width: fillWindowButton.width
            visible: wideScreen
        }
        ToolButton {
            id: copyNoteButton
            icon.name: "edit-copy"
            text: i18nc("@action:button", "Copy Note")
            display: AbstractButton.IconOnly
            onClicked: {
                document.copyWholeNote();
                copyMessage.visible = true;
                copyMessageTimer.restart();
            }
            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        ToolButton {
            id: fillWindowButton
            property int columnWidth: Config.fillWindow? 0 : Kirigami.Units.gridUnit * 15

            Behavior on columnWidth {
                NumberAnimation {

                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
            onColumnWidthChanged: pageStack.defaultColumnWidth = columnWidth
            visible: !root.singleDocumentMode && !Kirigami.Settings.isMobile
            icon.name: "view-fullscreen"
            text: i18n("Focus Mode")
            display: AbstractButton.IconOnly
            checkable: true
            checked: Config.fillWindow
            onClicked: {
                Config.fillWindow = !Config.fillWindow
            }
            Shortcut {
                sequence: "Ctrl+R"
                onActivated: Config.fillWindow = !Config.fillWindow
            }
            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            icon.name: "search"
            text: i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: true
            checkable: true
            checked: searchBar.isSearchOpen
            onClicked:
            {
                if(searchBar.isSearchOpen === true)
                {
                    root.closeSearch()
                }
                else
                {
                    root.openSearch()
                }
            }

            ToolTip.text: i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            visible: applicationWindow().visibility === Window.FullScreen
            icon.name: "window-restore-symbolic"
            text: i18nc("@action:menu", "Exit Full Screen")
            display: AbstractButton.IconOnly
            checkable: true
            checked: true
            onClicked: applicationWindow().showNormal()

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            icon.name: "view-list-details"
            text: i18nc("@action:button", "Table of Content")
            display: AbstractButton.IconOnly
            checkable: true
            checked: tocDrawer.opened
            onClicked: tocDrawer.opened ? tocDrawer.close() : tocDrawer.open()
            visible: true

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
        onAccepted: {
            if (imagePath.toString().length > 0) {
                document.insertImage(imagePath)
                imagePath = '';
            }
        }
        notePath: root.noteFullPath
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
                Shortcut {
                    sequence: StandardKey.Bold
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
                    sequence: StandardKey.Italic
                    onActivated: italicButton.clicked()
                }
                icon.name: "format-text-italic"
                text: i18nc("@action:button", "Italic")
                display: AbstractButton.IconOnly
                checkable: true
                checked: document.italic
                onClicked: {
                    document.italic = !document.italic;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                id: underlineButton
                Shortcut {
                    sequence: StandardKey.Underline
                    onActivated: underlineButton.clicked()
                }
                icon.name: "format-text-underline"
                text: i18nc("@action:button", "Underline")
                display: AbstractButton.IconOnly
                checkable: true
                checked: document.underline
                onClicked: {
                    document.underline = !document.underline;
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
                    document.strikethrough = !document.strikethrough;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }

    Kirigami.Action {
        id: indentAction

        text: i18nc("@action:button", "Increase List Level")
        icon.name: "format-indent-more"
        onTriggered: {
            document.indentListMore();
        }
        enabled: root.listIndent
    }

    Kirigami.Action {
        id: dedentAction
        icon.name: "format-indent-less"
        text: i18nc("@action:button", "Decrease List Level")
        onTriggered: {
            document.indentListLess();
        }
        enabled: root.listDedent
    }

    Component {
        id: listFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                action: indentAction
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                action: dedentAction
                display: AbstractButton.IconOnly
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

    Components.FloatingButton {
        icon.name: "document-edit"
        parent: root.overlay
        visible: !wideScreen
        scale: mobileToolBarContainer.hidden? 1 : 0

        Behavior on scale {
            NumberAnimation {

                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit
            bottomMargin: Kirigami.Units.gridUnit

        }

        onClicked: mobileToolBarContainer.hidden = false

    }

    RowLayout {
        id: mobileToolBarContainer
        visible: !wideScreen
        property bool hidden: false
        y: hidden? parent.height : parent.height - mobileToolBar.height

        anchors {
            left: parent.left
            right: parent.right
        }

        z: 600000
        parent: root.overlay

        Behavior on y {
            NumberAnimation {

                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        Kirigami.ShadowedRectangle {
            id: mobileToolBar

            Layout.fillHeight: true
            Layout.fillWidth: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.gridUnit * 5 + Kirigami.Units.smallSpacing*2

            shadow {
                size: 15
                color: Qt.rgba(0, 0, 0, 0.2)
            }
            MouseArea {
                anchors.fill: parent
            }
            Kirigami.Separator {
                width: parent.width
                anchors.top: parent.top

            }

            ColumnLayout {
                id: mobileToolbarLayout


                anchors.fill: parent
                RowLayout{

                    SwipeView {
                    id: swipeView
                    clip: true
                    Layout.margins: Kirigami.Units.mediumSpacing
                    Layout.fillWidth: true
                    implicitHeight: undoButton.height + Kirigami.Units.smallSpacing
                    currentIndex: categorySelector.selectedIndex
                    interactive: false
                    Item {
                        id: firstPage

                        RowLayout {
                            width: swipeView.width
                            height: swipeView.height
                            Loader {
                                sourceComponent: textFormatGroup
                                active: !root.wideScreen // Only active on mobile
                            }
                            Item { Layout.fillWidth: true }
                            Loader { sourceComponent: headingGroup }
                        }
                    }
                    Item {
                        id: secondPage
                        RowLayout {
                            height: swipeView.height
                            width: swipeView.width
                            Loader { sourceComponent: listFormatGroup }
                            Item { Layout.fillWidth: true }
                            Loader { sourceComponent: listStyleGroup }
                        }                    }
                    Item {
                        id: thirdPage
                        RowLayout {
                            height: swipeView.height
                            width: swipeView.width
                            Loader { sourceComponent: insertGroup }
                        }
                    }

                }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                        Layout.topMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.mediumSpacing
                    }
                    ToolButton {
                        icon.name: "edit-undo"
                        text: i18n("Undo")
                        display: AbstractButton.IconOnly
                        onClicked: textArea.undo()
                        enabled: textArea.canUndo
//                        Layout.topMargin: Kirigami.Units.smallSpacing
//                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    ToolButton {
                        id: undoButton
                        icon.name: "edit-redo"
                        text: i18n("Redo")
                        display: AbstractButton.IconOnly
                        onClicked: textArea.redo()
                        enabled: textArea.canRedo
//                        Layout.topMargin: Kirigami.Units.smallSpacing
//                        Layout.bottomMargin: Kirigami.Units.smallSpacing

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                }
                RowLayout {
                    Layout.fillWidth: true
                    Item{
                        Layout.fillWidth: true
                    }
                    Components.RadioSelector {
                        id: categorySelector

                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.topMargin: 0
                        Layout.fillWidth: true
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                        Layout.alignment: Qt.AlignHCenter

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
                    Item{
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.name: "arrow-down"
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.rightMargin: Kirigami.Units.mediumSpacing
                        icon.height: Kirigami.Units.gridUnit
                        icon.width: Kirigami.Units.gridUnit
                        Layout.alignment: Qt.AlignRight

                        Layout.topMargin: 0
                        height: categorySelector.height
                        width: height

                        onClicked: mobileToolBarContainer.hidden = true

                    }
                }
            }
        }
    }

    TocModel {
        id: tocModel
        document: textArea.textDocument
    }

    Kirigami.OverlayDrawer {
        id: tocDrawer
        edge: Qt.RightEdge
        modal: false
        handleVisible: false

        width: Kirigami.Units.gridUnit * 15
        
        parent: applicationWindow().overlay

        topMargin: (typeof pageStack !== "undefined" && pageStack.globalToolBar) ? pageStack.globalToolBar.height : (applicationWindow().header ? applicationWindow().header.height : 0)
        bottomMargin: toolBar.visible ? (toolBar.height + Kirigami.Units.largeSpacing * 2) : (mobileToolBarContainer.visible && !mobileToolBarContainer.hidden ? mobileToolBarContainer.height : 0)

        height: parent.height - topMargin - bottomMargin

        Component.onCompleted: tocDrawer.close()

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: i18nc("@title:window", "Table of Contents")
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    type: Kirigami.Heading.Type.Primary
                }

                ToolButton {
                    icon.name: "dialog-close"
                    text: i18nc("@action:button", "Close")
                    display: AbstractButton.IconOnly
                    onClicked: tocDrawer.close()

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }

            ListView {
                id: tocListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: tocModel
                clip: true

                delegate: ItemDelegate {
                    id: tocDelegate
                    width: ListView.view.width

                    required property string title
                    required property int level
                    required property int index
                    required property int cursorPosition

                    text: title
                    leftPadding: (level - 1) * Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                    highlighted: ListView.isCurrentItem

                    onClicked: {
                        ListView.view.currentIndex = index
                        textArea.cursorPosition = cursorPosition
                        textArea.forceActiveFocus()
                        if (Kirigami.Settings.isMobile) {
                            tocDrawer.close()
                        }
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    icon.name: "format-list-unordered"
                    visible: tocListView.count === 0
                    text: i18n("No headers found")
                }
            }
        }
    }

    Components.FloatingToolBar {
        id: toolBar

        visible: wideScreen
        z: 600000
        parent: root.overlay

        anchors {
            bottom: parent.bottom
            margins: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        contentItem: RowLayout {
            Loader {
                sourceComponent: textFormatGroup
                active: root.wideScreen // Only active on desktop
            }
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

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    header: ColumnLayout {
        spacing: 0

	    ToolBar {
            id: searchBar

            property bool isSearchOpen: false

            visible: isSearchOpen || opacity > 0
            opacity: isSearchOpen ? 1.0 : 0.0
            Behavior on opacity {
                NumberAnimation { duration: Kirigami.Units.shortDuration * 2}
            }

            Layout.preferredHeight: isSearchOpen ? implicitHeight : 0
            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuad
                }
            }

            clip: true

            Layout.fillWidth: true

            contentItem: RowLayout {
                id: searchBarLayout
                spacing: Kirigami.Units.smallSpacing

                Kirigami.SearchField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: i18n("Find text...")
                    onTextChanged: {
                        if (text.length > 0) {
                            document.findText(text);
                        } else {
                            document.clearSearch();
                            textArea.deselect();
                        }
                    }
                    Keys.onShortcutOverride: (event)=> event.accepted = (event.key === Qt.Key_Escape)
                    Keys.onReturnPressed: document.findNext()
                    Keys.onEscapePressed: {
                        searchField.text = "";
                        searchBar.isSearchOpen = false;
                        textArea.deselect();
                        textArea.forceActiveFocus();
                    }
                }

                Kirigami.Separator {
                    Layout.fillHeight: true
                }

                ToolButton {
                    icon.name: "go-up"
                    text: i18n("Previous")
                    display: AbstractButton.IconOnly
                    onClicked: document.findPrevious()
                    enabled: document.searchMatchCount > 0

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                    Shortcut {
                        sequence: StandardKey.FindPrevious
                        onActivated: document.findPrevious()
                    }
                }

                ToolButton {
                    icon.name: "go-down"
                    text: i18n("Next")
                    display: AbstractButton.IconOnly
                    onClicked: document.findNext()
                    enabled: document.searchMatchCount > 0

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                    Shortcut {
                        sequence: StandardKey.FindNext
                        onActivated: document.findNext()
                    }
                }

                Label {
                    text: {
                        if (document.searchMatchCount === 0) {
                            textArea.deselect();
                            return i18n("No matches");
                        }
                        return i18n("%1/%2", document.searchCurrentMatch + 1, document.searchMatchCount);
                    }
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 4
                }

                ToolButton {
                    icon.name: "dialog-close"
                    text: i18n("Close")
                    display: AbstractButton.IconOnly
                    onClicked: {
                        root.closeSearch()
                    }

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }

        Kirigami.InlineMessage {
            id: copyMessage

            text: i18nc("@info:status", "The note has been copied to the clipboard.")
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Positive
            visible: false
            showCloseButton: true

            Layout.fillWidth: true
        }
    }

    contentItem: ScrollView {
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Layout.fillWidth: true
        Layout.fillHeight: true

        bottomPadding: wideScreen ? 0 : (mobileToolBarContainer.hidden ? 0 : mobileToolBarContainer.height)

        // Animate scroll bar between wide and mobile screens transitions
        Behavior on bottomPadding {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutInQuart
            }
        }

        T.TextArea {
            id: textArea

            textMargin: !Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: !Kirigami.Settings.isMobile ? (toolBar.height + Kirigami.Units.largeSpacing * 2) : Kirigami.Units.smallSpacing

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutInQuart
                }
            }

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

            property int lastKey: -1
            Keys.onPressed: (event) => {
                lastKey = event.key
            }

            Shortcut {
                sequence: "Ctrl+E"
                onActivated:
                {
                    if(searchBar.isSearchOpen === true)
                    {
                        root.closeSearch()
                    }
                    else
                    {
                        root.openSearch()
                    }
                }
            }

            onTextChanged: {
                if (lastKey !== -1) {
                    let key = lastKey;
                    lastKey = -1;
                    document.slotKeyPressed(key);
                }
                root.saved = false;
                saveTimer.restart()
            }
            persistentSelection: true
            height: parent.height
            textFormat: TextEdit.MarkdownText
            wrapMode: TextEdit.Wrap

            TableActionHelper {
                id: tableHelper

                document: textArea.textDocument
                cursorPosition: textArea.cursorPosition
                selectionStart: textArea.selectionStart
                selectionEnd: textArea.selectionEnd
            }

            DropArea {
                id: imageDropArea
                anchors.fill: parent

                onEntered: (drag) => {
                    let compatible = false;
                    for (let i = 0; i < drag.formats.length; i++) {
                        const fmt = drag.formats[i].toString();
                        // Allow text/uri-list as some file managers use this format
                        if (fmt.indexOf("image/") === 0 || fmt === "text/uri-list") {
                            compatible = true;
                            break;
                        }
                    }

                    if (compatible) {
                        drag.acceptProposedAction()
                    }
                }

                onDropped: (drop) => {
                    if (drop.hasUrls) {
                        for (let i = 0; i < drop.urls.length; i++) {
                            const path = drop.urls[i].toString();
                            document.insertImage(path);
                        }
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
                        if (root.noteFullPath.toString().length > 0) {
                            document.load(root.noteFullPath);
                            root.saved = true;
                            root.oldPath = root.noteFullPath;
                            textArea.forceActiveFocus();
                        }
                        applicationWindow().currentDocument = document;
                    }

                    Component.onDestruction: {
                        if (!saved && root.noteFullPath.toString().length > 0) {
                            document.saveAs(root.noteFullPath);
                        }
                        if (applicationWindow().currentDocument === document) {
                            applicationWindow().currentDocument = null;
                        }
                    }

                    onCheckableChanged: {
                        root.checkbox = document.checkable;
                    }

                    onMoveCursor: (position) => {
                        textArea.cursorPosition = position;
                    }
                    onSelectCursor: (start, end) => {
                        textArea.select(start, end);
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
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                // unfortunately, taphandler's pressed event only triggers when the press is lifted
                // we need to use the longpress signal since it triggers when the button is first pressed
                longPressThreshold: 0.001 // https://invent.kde.org/qt/qt/qtdeclarative/-/commit/8f6809681ec82da783ae8dcd76fa2c209b28fde6
                onLongPressed: {
                    textFieldContextMenu.currentLink = document.anchorAt(point.position);
                    textFieldContextMenu.targetClick(
                        point,
                        textArea,
                        /*spellcheckHighlighterInstantiator*/ null,
                        /*mousePosition*/ null,
                    );
                }
            }

            Timer {
                id: saveTimer

                repeat: false
                interval: 1000
                onTriggered: if (root.noteFullPath.toString().length > 0) {
                    document.saveAs(root.noteFullPath);
                    saved = true;
                }
            }
        }
    }

    Timer {
        id: copyMessageTimer
        interval: 3000
        repeat: false
        onTriggered: copyMessage.visible = false
    }

    TextFieldContextMenu {
        id: textFieldContextMenu
        tableActionHelper: tableHelper
        document: document
    }
}
