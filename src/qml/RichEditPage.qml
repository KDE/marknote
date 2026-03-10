// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts
import org.kde.ki18n

import "components"

import org.kde.kirigamiaddons.components as Components
import org.kde.marknote

EditPage {
    id: root

    property bool listIndent: true
    property bool listDedent: true
    property bool checkbox: false
    property int listStyle: 0
    property int heading: 0

    readonly property bool canFitToolbar: width >= toolBar.width + Kirigami.Units.largeSpacing * 2 && tocLoader.active !== true

    mobileToolBarHidden: mobileToolBarContainer.hidden
    mobileToolBarHeight: mobileToolBarContainer.height

    dynamicRightPadding: tocLoader.item ? (tocLoader.item.position * tocLoader.item.width) : 0

    objectName: "RichEditPage"

    document: RichDocumentHandler {
        textArea: root.textArea
        document: root.textArea.textDocument
        cursorPosition: root.textArea.cursorPosition
        selectionStart: root.textArea.selectionStart
        selectionEnd: root.textArea.selectionEnd


        onLoaded: (text) => {
            root.textArea.text = text
        }
        onError: (message) => {
            console.error("Error message from document handler", message)
        }

        onCopy: root.textArea.copy();
        onCut: root.textArea.cut();
        onUndo: root.textArea.undo();
        onRedo: root.textArea.redo();


        onCheckableChanged: {
            root.checkbox = checkable;
        }

        onMoveCursor: (position) => {
            root.textArea.cursorPosition = position;
        }
        onSelectCursor: (start, end) => {
            root.textArea.select(start, end);
        }

        onCursorPositionChanged: {
            root.listIndent = canIndentList;
            root.listDedent = canDedentList;
            root.checkbox = checkable;

            if (currentListStyle === 0) {
                root.listStyle = 0;
            } else if (currentListStyle === 1) {
                root.listStyle = 1;
            } else if (currentListStyle === 4) {
                root.listStyle = 2;
            }
            root.heading = currentHeadingLevel
        }

        onInternalLinkActivated: (noteName) => {
            root.openNoteByName(noteName);
        }
    }

    NoteBooksModel {
        id: allNotebooksModel
        storagePath: Config.storage
    }

    NotesModel {
        id: notesSearchModel
        path: NavigationController.notebookPath
    }

    function normalizeNoteName(name: string): string {
        if (!name) {
            return "";
        }
        let normalized = name;
        if (normalized.endsWith(".md")) {
            normalized = normalized.slice(0, -3);
        }
        return normalized.trim();
    }

    function ensureNotebookPath(): string {
        if (NavigationController.notebookPath.length > 0) {
            return NavigationController.notebookPath;
        }
        if (allNotebooksModel.rowCount() === 0) {
            return "";
        }
        const firstIndex = allNotebooksModel.index(0, 0);
        return allNotebooksModel.data(firstIndex, NoteBooksModel.Path);
    }

    function findNoteNotebookPath(noteName: string): string {
        const normalized = normalizeNoteName(noteName);
        if (!normalized) {
            return "";
        }
        const total = allNotebooksModel.rowCount();
        for (let i = 0; i < total; i++) {
            const idx = allNotebooksModel.index(i, 0);
            const notebookPath = allNotebooksModel.data(idx, NoteBooksModel.Path);
            if (!notebookPath) {
                continue;
            }
            notesSearchModel.path = notebookPath;
            if (notesSearchModel.noteExists(normalized)) {
                return notebookPath;
            }
        }
        return "";
    }

    function openNoteByName(name: string): void {
        const normalized = normalizeNoteName(name);
        if (!normalized) {
            return;
        }

        const foundNotebookPath = findNoteNotebookPath(normalized);
        if (foundNotebookPath.length > 0) {
            if (NavigationController.notebookPath !== foundNotebookPath) {
                NavigationController.notebookPath = foundNotebookPath;
            }
            NavigationController.notePath = normalized + ".md";
            return;
        }

        const targetNotebookPath = ensureNotebookPath();
        if (!targetNotebookPath.length) {
            return;
        }
        if (NavigationController.notebookPath !== targetNotebookPath) {
            NavigationController.notebookPath = targetNotebookPath;
        }
        notesSearchModel.path = targetNotebookPath;
        if (!notesSearchModel.noteExists(normalized)) {
            notesSearchModel.addNote(normalized);
        }
        NavigationController.notePath = normalized + ".md";
    }

    function noteNameFromInternalUrl(url): string {
        if (!url) {
            return "";
        }
        const urlString = url.toString();
        const prefix = "marknote://note/";
        if (!urlString.startsWith(prefix)) {
            return "";
        }
        const encodedName = urlString.substring(prefix.length);
        return decodeURIComponent(encodedName);
    }

    function openInternalLinkUrl(url): void {
        const noteName = noteNameFromInternalUrl(url);
        if (noteName.length > 0) {
            openNoteByName(noteName);
        }
    }

    titleDelegate: RowLayout {
        visible: root.noteName
        Layout.fillWidth: true
        ToolButton {
            icon.name: "edit-undo"
            text: KI18n.i18n("Undo")
            display: AbstractButton.IconOnly
            Layout.leftMargin: Kirigami.Units.smallSpacing
            onClicked: root.textArea.undo()
            enabled: root.textArea.canUndo
            visible: root.isWideScreen && !root.singleDocumentMode && !mobileToolbarLayout.visible

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            icon.name: "edit-redo"
            text: KI18n.i18n("Redo")
            display: AbstractButton.IconOnly
            onClicked: root.textArea.redo()
            enabled: root.textArea.canRedo
            visible: root.isWideScreen && !root.singleDocumentMode && !mobileToolbarLayout.visible

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            Layout.preferredWidth: 5
            Layout.preferredHeight: 5
            radius: 5
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
            text: tocLoader.active && !root.singleDocumentMode && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn ? tocButton.text : root.noteName
            elide: Text.ElideRight
            wrapMode: Text.NoWrap

            Layout.rightMargin: Kirigami.Units.mediumSpacing
            Layout.leftMargin: Kirigami.Units.mediumSpacing
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth + Kirigami.Units.mediumSpacing * 2
            Layout.minimumWidth: 0
        }


        Item { Layout.fillWidth: true }
        Item {
            Layout.preferredWidth: fillWindowButton.width
            visible: root.isWideScreen
        }
        ToolButton {
            id: searchNoteButton
            icon.name: "search"
            text: KI18n.i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: true
            checkable: true
            checked: root.searchBar.isSearchOpen
            onClicked: if (root.searchBar.isSearchOpen === true) {
                root.closeSearch()
            } else {
                root.openSearch()
            }

            ToolTip.text: KI18n.i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }


        ToolButton {
            id: tocButton
            icon.name: "view-list-details"
            text: KI18n.i18nc("@action:button", "Table of Content")
            display: AbstractButton.IconOnly
            checkable: true
            checked: tocLoader.active
            onClicked: tocLoader.active ? tocLoader.close() : tocLoader.open()
            visible: true

            ToolTip.text: KI18n.i18nc("@info:tooltip", tocButton.text)
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            id: fillWindowButton
            property int columnWidth: Config.fillWindow? 0 : Kirigami.Units.gridUnit * 15

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            checkable: true
            checked: Config.fillWindow
            display: AbstractButton.IconOnly
            icon.name: "view-fullscreen"
            text: KI18n.i18n("Focus Mode")
            visible: (root.isWideScreen || Config.fillWindow) && !root.singleDocumentMode && !Kirigami.Settings.isMobile

            Behavior on columnWidth {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
            onColumnWidthChanged: root.pageStack.defaultColumnWidth = columnWidth

            onClicked: {
                Config.fillWindow = !Config.fillWindow
            }
        }

        ToolButton {
            visible: root.Window.window.visibility === Window.FullScreen
            icon.name: "window-restore-symbolic"
            text: KI18n.i18nc("@action:menu", "Exit Full Screen")
            display: AbstractButton.IconOnly
            checkable: true
            checked: true
            onClicked: root.Window.window.showNormal()

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Button{
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: KI18n.i18n("Switch editor to source mode")
            ToolTip.visible: hovered
            icon.name: "code-context-symbolic"
            checkable: true
            checked: false
            text: KI18n.i18n("Source View")
            padding: 0
            flat: true
            spacing: Kirigami.Units.mediumSpacing

            display: AbstractButton.IconOnly

            onClicked: {
                document.saveAs(root.noteFullPath)
                NavigationController.sourceMode = !NavigationController.sourceMode
            }
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            visible: tocLoader.active && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns
        }

        RowLayout {
            visible: tocLoader.active && !root.isNarrow && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            readonly property real exactWidth: (Kirigami.Units.gridUnit * 15) - Kirigami.Units.largeSpacing

            Layout.preferredWidth: exactWidth
            Layout.maximumWidth: exactWidth
            Layout.minimumWidth: exactWidth

            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: KI18n.i18nc("@title:window", "Table of Contents")
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }
    }

    LinkDialog {
        id: linkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.Overlay.overlay
        onAccepted: root.document.updateLink(linkUrl, linkText)
    }

    NoteLinkDialog {
        id: noteLinkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: ApplicationWindow.window.overlay
        onAccepted: document.updateNoteLink(noteName, noteAlias)
    }

    ImageDialog {
        id: imageDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.Overlay.overlay
        onAccepted: {
            if (imagePath.toString().length > 0) {
                root.document.insertImage(imagePath)
                imagePath = '';
            }
        }
        notePath: root.noteFullPath
    }

    TableDialog {
        id: tableDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.Overlay.overlay
        onAccepted: root.document.insertTable(rows, cols)
    }

    Loader {
        id: tocLoader

        active: false

        function open(): void {
            tocLoader.active = true;
        }

        function close(): void {
            item.closed.connect(() => tocLoader.active = false);
            item.close();
        }

        sourceComponent: TocDrawer {
            textArea: root.textArea
            parent: root.Overlay.overlay

            topMargin: (root.pageStack && root.pageStack.globalToolBar) ? root.pageStack.globalToolBar.height : (root.ApplicationWindow.window && root.ApplicationWindow.window.header ? root.ApplicationWindow.window.header.height : 0)
            bottomMargin: 0

            height: parent.height - topMargin
        }
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
                text: KI18n.i18nc("@action:button", "Bold")
                display: AbstractButton.IconOnly
                checkable: true

                checked: root.document.bold ?? false

                onClicked: {
                    root.document.bold = !root.document.bold
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
                text: KI18n.i18nc("@action:button", "Italic")
                display: AbstractButton.IconOnly
                checkable: true
                checked: root.document.italic ?? false
                onClicked: {
                    root.document.italic = !root.document.italic;
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
                text: KI18n.i18nc("@action:button", "Underline")
                display: AbstractButton.IconOnly
                checkable: true
                checked: root.document.underline ?? false
                onClicked: {
                    root.document.underline = !root.document.underline;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                icon.name: "format-text-strikethrough"
                text: KI18n.i18nc("@action:button", "Strikethrough")
                display: AbstractButton.IconOnly
                checkable: true
                checked: root.document.strikethrough ?? false
                onClicked: {
                    root.document.strikethrough = !root.document.strikethrough;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }

    Kirigami.Action {
        id: indentAction

        text: KI18n.i18nc("@action:button", "Increase List Level")
        icon.name: "format-indent-more"
        onTriggered: {
            root.document.indentListMore();
        }
        enabled: root.listIndent
    }

    Kirigami.Action {
        id: dedentAction
        icon.name: "format-indent-less"
        text: KI18n.i18nc("@action:button", "Decrease List Level")
        onTriggered: {
            root.document.indentListLess();
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
                root.document.setListStyle(currentValue);
            }
            currentIndex: root.listStyle ?? 0
            enabled: indentAction.enabled || dedentAction.enabled
            textRole: "text"
            valueRole: "value"
            model: [
                { text: KI18n.i18nc("@item:inmenu no list style", "No list"), value: 0 },
                { text: KI18n.i18nc("@item:inmenu unordered style", "Unordered list"), value: 1 },
                { text: KI18n.i18nc("@item:inmenu ordered style", "Ordered list"), value: 4 },
            ]
        }
    }
    Component{
        id: insertGroup

        RowLayout {
            ToolButton {
                id: checkboxAction
                icon.name: "checkbox-symbolic"
                text: KI18n.i18nc("@action:button", "Insert checkbox")
                display: AbstractButton.IconOnly
                checkable: true
                onClicked: {
                    root.document.checkable = !root.document.checkable;
                }
                checked: root.checkbox
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                id: linkAction
                icon.name: "insert-link-symbolic"
                text: KI18n.i18nc("@action:button", "Insert link")
                display: AbstractButton.IconOnly
                onClicked: {
                    linkDialog.linkText = root.document.currentLinkText();
                    linkDialog.linkUrl = root.document.currentLinkUrl();
                    linkDialog.open();
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                id: noteLinkAction
                icon.name: "text-frame-link-symbolic"
                text: i18nc("@action:button", "Insert note link")
                display: AbstractButton.IconOnly
                onClicked: {
                    noteLinkDialog.noteAlias = root.document.currentNoteLinkAlias();
                    noteLinkDialog.noteName = root.document.currentNoteLinkName();
                    noteLinkDialog.open();
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                id: imageAction
                icon.name: "insert-image-symbolic"
                text: KI18n.i18nc("@action:button", "Insert image")
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
                text: KI18n.i18nc("@action:button", "Insert table")
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

    Component {
        id: headingGroup
        ComboBox {
            id: headingLevelComboBox
            currentIndex: root.heading ?? 0

            model: [
                KI18n.i18nc("@item:inmenu no heading", "Basic text"),
                KI18n.i18nc("@item:inmenu heading level 1 (largest)", "Title"),
                KI18n.i18nc("@item:inmenu heading level 2", "Subtitle"),
                KI18n.i18nc("@item:inmenu heading level 3", "Section"),
                KI18n.i18nc("@item:inmenu heading level 4", "Subsection"),
                KI18n.i18nc("@item:inmenu heading level 5", "Paragraph"),
                KI18n.i18nc("@item:inmenu heading level 6 (smallest)", "Subparagraph")
            ]

            onActivated: (index) => {
                root.document.setHeadingLevel(index);
            }
        }
    }

    Components.FloatingButton {
        icon.name: "document-edit"
        parent: root.overlay
        visible: !root.isWideScreen && !NavigationController.sourceMode
        scale: mobileToolBarContainer.hidden ? true : false

        property int defaultSpacing: Kirigami.Units.largeSpacing * 2
        property T.ScrollBar verticalScrollBar: root.contentScroll.ScrollBar.vertical

        Behavior on scale {
            NumberAnimation {

                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: verticalScrollBar.visible ?
                         defaultSpacing + verticalScrollBar.width  :
                         defaultSpacing
            bottomMargin: defaultSpacing
        }

        onClicked: mobileToolBarContainer.hidden = false

    }

    RowLayout {
        id: mobileToolBarContainer

        property bool hidden: NavigationController.sourceMode

        visible: !root.canFitToolbar
        y: hidden ? parent.height : parent.height - mobileToolBar.height

        anchors {
            left: parent.left
            right: parent.right
            rightMargin: root.dynamicRightPadding
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
            Layout.preferredHeight: Kirigami.Units.gridUnit * 5 + Kirigami.Units.smallSpacing*2

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

                RowLayout {
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
                                    active: !root.canFitToolbar // Only active on mobile
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
                            }
                        }
                        
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
                        text: KI18n.i18n("Undo")
                        display: AbstractButton.IconOnly
                        onClicked: root.textArea.undo()
                        enabled: root.textArea.canUndo
                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    ToolButton {
                        id: undoButton
                        icon.name: "edit-redo"
                        text: KI18n.i18n("Redo")
                        display: AbstractButton.IconOnly
                        onClicked: root.textArea.redo()
                        enabled: root.textArea.canRedo

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                }

                RowLayout {
                    Layout.fillWidth: true

                    Item{ Layout.fillWidth: true }

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
                               text: KI18n.i18n("Format")
                                //icon.name: "format-border-style"
                           },
                           Kirigami.Action {
                               text: KI18n.i18n("Lists")
                                //icon.name: "media-playlist-append"
                           },
                           Kirigami.Action {
                               text: KI18n.i18n("Insert")
                                // icon.name: "kdenlive-add-text-clip"
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
                        Layout.preferredWidth: categorySelector.height
                        Layout.preferredHeight: categorySelector.height

                        onClicked: mobileToolBarContainer.hidden = true

                    }
                }
            }
        }
    }

    Components.FloatingToolBar {
        id: toolBar

        visible: root.canFitToolbar && !NavigationController.sourceMode
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
                active: root.canFitToolbar // Only active on desktop
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
         
    Timer {
        id: copyMessageTimer
        interval: 3000
        repeat: false
        onTriggered: root.copyMessage.visible = false
    }

    textFieldContextMenu: TextFieldContextMenu {
        tableActionHelper: TableActionHelper {
            id: tableHelper

            document: root.textArea.textDocument
            cursorPosition: root.textArea.cursorPosition
            selectionStart: root.textArea.selectionStart
            selectionEnd: root.textArea.selectionEnd
        }
        document: root.document
        onInternalLinkClicked: (link) => root.openInternalLinkUrl(link)
    }
}

