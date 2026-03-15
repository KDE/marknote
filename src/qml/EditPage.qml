// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts
import org.kde.ki18n

import "components"

import org.kde.marknote

Kirigami.Page {
    id: root

    readonly property bool isWideScreen: !!ApplicationWindow.window?.isWideScreen

    property bool init: false

    property string noteFullPath: NavigationController.noteFullPath
    property string noteName: NavigationController.noteName

    property string oldPath: ''
    property bool saved: true
    property bool singleDocumentMode: false
    property real dynamicRightPadding: 0

    readonly property alias textArea: textArea
    readonly property alias copyMessage: copyMessage
    required property var document
    readonly property alias contentScroll: contentScroll
    property alias searchBar: searchBar 
    readonly property Kirigami.PageRow pageStack: (root.ApplicationWindow.window as Kirigami.ApplicationWindow)?.pageStack ?? null
    required property TextFieldContextMenu textFieldContextMenu

    property bool mobileToolBarHidden: true
    property real mobileToolBarHeight: 0

    property bool supportsToc: false
    property bool isTocOpened: false
    property real tocPosition: 0

    onWidthChanged: {
        // 30 grid units gives enough room for the 15-unit drawer + 15 units of text
        // Have nothing to do in the source mode
        if (!NavigationController.sourceMode &&
            tocDrawer.opened &&
            width < (tocDrawer.width + Kirigami.Units.gridUnit * 15)) {
            tocDrawer.close()
            }
    }

    function openSearch(): void {
        if (searchBar) {
            searchBar.isSearchOpen = true;
            if (searchField) {
                let selText = textArea.selectedText;
                if (selText.length > 0) {
                    // Strip out any carriage returns/newlines
                    searchField.text = selText.replace(/[\r\n]+/g, " ").trim();
                }

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
            searchBar.isReplaceVisible = false;
            textArea.deselect();

            if (textArea) {
                Qt.callLater(function() {
                    textArea.forceActiveFocus();
                });
            }
        }
    }

    function loadNote(): void {
        if (!root.visible){
            return;
        }
        if (root.oldPath.length > 0 && !saved) {
            root.document.saveAs(root.oldPath);
        }
        if (root.noteFullPath.toString().length > 0) {
        
            root.document.load(root.noteFullPath);

            root.saved = true;
        }

        root.oldPath = root.noteFullPath;

        textArea.forceActiveFocus();
    }

    Layout.fillWidth: true

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    bottomPadding: 0
    leftPadding: 0
    rightPadding: dynamicRightPadding
    topPadding: 0

    function toggleSearch(): void
    {
        if (searchBar.isSearchOpen) {
            root.closeSearch()
        } else {
            root.openSearch()
        }
    }

    function toggleReplace(): void {
        searchBar.isReplaceVisible = !searchBar.isReplaceVisible

        if (!searchBar.isSearchOpen) {
            root.openSearch()
        } else {
            searchBar.isReplaceVisible ? replaceField.forceActiveFocus() : searchField.forceActiveFocus()
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
            visible: root.isWideScreen && !root.singleDocumentMode

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
            visible: root.isWideScreen && !root.singleDocumentMode

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            color: Kirigami.Theme.textColor
            Layout.preferredHeight: 5
            Layout.preferredWidth: 5
            radius: Kirigami.Units.cornerRadius
            scale: root.saved ? 0 : 1

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
        }

        Kirigami.Heading {
            text: root.isTocOpened && !root.singleDocumentMode && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn ?
            KI18n.i18nc("@action:button", "Table of Content") : root.noteName
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
            visible: root.isWideScreen
            Layout.preferredWidth: fillWindowButton.width
        }

        ToolButton {
            icon.name: "search"
            text: KI18n.i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: true
            checkable: true
            checked: root.searchBar.isSearchOpen

            onClicked: toggleSearch()

            ToolTip.text: KI18n.i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        ToolButton {
            icon.name: "view-list-details"
            text: KI18n.i18nc("@action:button", "Table of Content")
            display: AbstractButton.IconOnly
            checkable: true
            checked: root.isTocOpened
            visible: root.supportsToc
            onClicked: root.toggleToc()

            Shortcut {
                sequence: "Ctrl+T"
                enabled: root.supportsToc
                onActivated: root.toggleToc()
            }

            ToolTip.text: KI18n.i18nc("@info:tooltip", text)
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
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
            text: KI18n.i18n("Focus Mode")
            visible: (root.isWideScreen || Config.fillWindow) && !root.singleDocumentMode && !Kirigami.Settings.isMobile

            Behavior on columnWidth {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            onClicked: Config.fillWindow = !Config.fillWindow
            onColumnWidthChanged: root.pageStack.defaultColumnWidth = columnWidth
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

        Button {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: KI18n.i18n("Switch editor mode")
            ToolTip.visible: hovered
            icon.name: "code-context-symbolic"
            checkable: true
            checked: NavigationController.sourceMode
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
            visible: root.tocPosition > 0 && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns
            opacity: root.tocPosition
        }

        RowLayout {
            visible: root.tocPosition > 0 && !root.isNarrow && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            readonly property real alignSeparatorWidth: root.contentScroll.ScrollBar.vertical.visible ? 15.7 : 14.6
            readonly property real fullWidth: (Kirigami.Units.gridUnit * alignSeparatorWidth) - Kirigami.Units.largeSpacing
            readonly property real exactWidth: fullWidth * root.tocPosition

            Layout.preferredWidth: exactWidth
            Layout.maximumWidth: exactWidth
            Layout.minimumWidth: exactWidth

            opacity: root.tocPosition
            clip: true
            spacing: 0

            Item { Layout.fillWidth: true }
            Kirigami.Heading {
                text: KI18n.i18nc("@title:window", "Table of Contents")
                elide: Text.ElideRight
            }
            Item { Layout.fillWidth: true }
        }
    }

    header: ColumnLayout {

        spacing: 0

	    ToolBar {
            id: searchBar

            property bool isSearchOpen: false
            property bool isReplaceVisible: false

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
            Layout.rightMargin: root.dynamicRightPadding

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    id: searchBarLayout
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.SearchField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: KI18n.i18n("Find text...")
                        onTextChanged: {
                            // Strip out any carriage returns/newlines
                            const cleanText = text.replace(/[\r\n]+/g, "").trim();

                            if (cleanText.length > 0) {
                                root.document.findText(cleanText);
                            } else {
                                root.document.clearSearch();
                                textArea.deselect();
                            }
                        }
                        Keys.onShortcutOverride: (event) => {
                            if (event.key === Qt.Key_Escape || event.matches(StandardKey.Find)) {
                                event.accepted = true;
                            }
                        }

                        Keys.onPressed: (event) => {
                            if (event.matches(StandardKey.Find)) {
                                root.closeSearch();
                                event.accepted = true;
                            }
                        }
                        Keys.onReturnPressed: root.document.findNext()
                        Keys.onEscapePressed: {
                            searchField.text = "";
                            searchBar.isSearchOpen = false;
                            searchBar.isReplaceVisible = false;
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
                        onClicked: root.document.findPrevious()
                        enabled: root.document.searchMatchCount > 0

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

                    Kirigami.Separator {
                        Layout.fillHeight: true
                    }

                    ToolButton {
                        id: replaceToggleButton
                        icon.name: searchBar.isReplaceVisible ? "arrow-up" : "edit-find-replace-symbolic"
                        text: i18n("Replace")
                        display: AbstractButton.IconOnly
                        onClicked: toggleReplace()

                        Shortcut {
                            sequence: StandardKey.Replace
                            enabled: root.visible
                            onActivated: toggleReplace()
                        }

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    ToolButton {
                        id: closeButton
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

                // Replace bar
                RowLayout {
                    id: replaceBarLayout
                    spacing: Kirigami.Units.smallSpacing
                    visible: searchBar.isReplaceVisible
                    Layout.fillWidth: true

                    TextField {
                        id: replaceField
                        Layout.fillWidth: true
                        placeholderText: i18n("Replace with...")
                        Keys.onShortcutOverride: (event)=> event.accepted = (event.key === Qt.Key_Escape)
                        Keys.onReturnPressed: (event) => {
                            if (event.modifiers & Qt.ControlModifier) {
                                const count = document.replaceAll(replaceField.text);
                                replaceMessage.text = i18ncp("@info:status", "Replaced %1 occurrence", "Replaced %1 occurrences", count);
                                replaceMessage.visible = true;
                            } else {
                                document.replaceCurrent(replaceField.text);
                            }

                            searchField.forceActiveFocus();
                            searchField.selectAll();
                        }
                        Keys.onEscapePressed: {
                            replaceField.text = ""
                            searchBar.isSearchOpen = true;
                            searchBar.isReplaceVisible = false;
                            searchField.forceActiveFocus();
                            searchField.selectAll();
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                    }

                    ToolButton {
                        icon.name: "document-swap"
                        text: i18n("Replace")
                        display: AbstractButton.IconOnly
                        onClicked: document.replaceCurrent(replaceField.text)
                        enabled: document.searchMatchCount > 0

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    ToolButton {
                        icon.name: "view-refresh"
                        text: i18n("Replace All")
                        display: AbstractButton.IconOnly
                        onClicked: {
                            const count = document.replaceAll(replaceField.text);
                            replaceMessage.text = i18ncp("@info:status", "Replaced %1 occurrence", "Replaced %1 occurrences", count);
                            replaceMessage.visible = true;
                        }
                        enabled: document.searchMatchCount > 0

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    Label {
                        text: {
                            if (document.searchMatchCount === 0) {
                                return i18n("No matches");
                            }
                            return i18n("%1/%2", document.searchCurrentMatch + 1, document.searchMatchCount);
                        }
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 4
                    }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                    }

                    // Spacer to align with replace toggle button
                    Item {
                        Layout.preferredWidth: replaceToggleButton.width
                    }

                    Item {
                        Layout.preferredWidth: closeButton.width
                    }
                }
            }
        }

        Kirigami.InlineMessage {
            id: replaceMessage

            type: Kirigami.MessageType.Positive
            position: Kirigami.InlineMessage.Position.Header
            showCloseButton: true

            Layout.fillWidth: true
            Layout.rightMargin: root.dynamicRightPadding

            visible: false

            Timer {
                id: replaceMessageTimer
                interval: 3000
                onTriggered: replaceMessage.visible = false
            }

            onVisibleChanged: {
                if (visible) {
                    replaceMessageTimer.restart();
                }
            }
        }

        Kirigami.InlineMessage {
            id: copyMessage

            text: KI18n.i18nc("@info:status", "The note has been copied to the clipboard.")
            position: Kirigami.InlineMessage.Position.Header
            type: Kirigami.MessageType.Positive
            visible: false
            showCloseButton: true

            Layout.fillWidth: true
            Layout.rightMargin: root.dynamicRightPadding

            Timer {
                id: copyMessageTimer
                interval: 3000
                onTriggered: copyMessage.visible = false
            }

            onVisibleChanged: {
                if (visible) {
                    copyMessageTimer.restart();
                }
            }
        }
    }

    contentItem: ScrollView {
        id: contentScroll
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Layout.fillWidth: true
        Layout.fillHeight: true

        bottomPadding: root.canFitToolbar ? 0 : root.mobileToolBarHeight

        // Animate scroll bar between wide and mobile screens transitions
        Behavior on bottomPadding {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutInQuart
            }
        }

        T.TextArea {
            id: textArea

            readonly property int marginMultiplier: 6
            readonly property bool applyWideScreenMargin: (root.isWideScreen && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns) || singleDocumentMode
            textMargin: applyWideScreenMargin ? Kirigami.Units.largeSpacing * marginMultiplier : Kirigami.Units.smallSpacing * marginMultiplier

            leftPadding: 0
            rightPadding: Kirigami.Units.largeSpacing
            topPadding: 0
            bottomPadding: 0

            Behavior on bottomPadding {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutInQuart
                }
            }

            HoverHandler {
                id: controlHoverHandler
                acceptedModifiers: Qt.ControlModifier

                onPointChanged: () => {
                    root.document.slotMouseMovedWithControl(controlHoverHandler.point.position)
                }

                onHoveredChanged: () => {
                    if (!controlHoverHandler.hovered) {
                        root.document.slotMouseMovedWithControlReleased()
                    }
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
            persistentSelection: true
            height: parent.height
            textFormat: NavigationController.sourceMode ? TextEdit.PlainText : TextEdit.MarkdownText
            wrapMode: TextEdit.Wrap

            onPressAndHold: (event) => {
                if (Kirigami.Settings.tabletMode && selectByMouse) {
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }

            Keys.onShortcutOverride: (event) => {
                if (event.matches(StandardKey.Find)) {
                    event.accepted = true;
                }
            }

            property int lastKey: -1
            Keys.onPressed: (event) => {
                if (event.matches(StandardKey.Paste)) {
                    if (root.document && typeof root.document.pasteFromClipboard === 'function') {
                        root.document.pasteFromClipboard();
                        event.accepted = true;
                        return;
                    }
                } else if (event.matches(StandardKey.Find)) {
                    toggleSearch();
                    event.accepted = true;
                    return;
                }

                lastKey = event.key;
                event.accepted = false;
            }

            onTextChanged: {
                if (!NavigationController.sourceMode) {
                    if (lastKey !== -1) {
                        let key = lastKey;
                        lastKey = -1;
                        root.document.slotKeyPressed(key);
                    }
                }
                root.saved = false;
                saveTimer.restart()

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
                            root.document.insertImage(path);
                        }
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                // unfortunately, taphandler's pressed event only triggers when the press is lifted
                // we need to use the longpress signal since it triggers when the button is first pressed
                longPressThreshold: 0.001 // https://invent.kde.org/qt/qt/qtdeclarative/-/commit/8f6809681ec82da783ae8dcd76fa2c209b28fde6
                onLongPressed: {
                    root.textFieldContextMenu.currentLink = root.document.anchorAt(point.position);
                    root.textFieldContextMenu.targetClick(
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
                    root.document.saveAs(root.noteFullPath);
                    root.saved = true;
                }
            }
        }
    }

    Component.onCompleted: {
        loadNote();
        init = true;
    }

    onDocumentChanged: {
        if (document && init) {
            loadNote();
            let mainWindow = ApplicationWindow.window as Main;
            if (mainWindow) {
                mainWindow.currentDocument = root.document;
            }
        }
    }

    onVisibleChanged: {
        if (!ApplicationWindow.window) {
            return;
        }

        let mainWindow = ApplicationWindow.window as Main;
        if (!mainWindow) {
            return; // Safely exit if this isn't the Main window
        }

        if (visible) {
            mainWindow.currentDocument = root.document
        } else if (mainWindow.currentDocument === root.document) {
            mainWindow.currentDocument = null
        }
    }

    Component.onDestruction: {
        let mainWindow = ApplicationWindow.window as Main;
        if (mainWindow && mainWindow.currentDocument === root.document) {
            mainWindow.currentDocument = null
        }
    }

    onNoteFullPathChanged: () => {
        if (!init) {
            return;
        }
        loadNote();
    }
}
