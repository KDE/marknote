// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
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

    readonly property bool isWideScreen: ApplicationWindow.window ? ApplicationWindow.window.isWideScreen : false

    property bool init: false

    property string noteFullPath: NavigationController.noteFullPath
    property string noteName: NavigationController.noteName

    property string oldPath: ''
    property bool saved: true
    property bool singleDocumentMode: false

    readonly property alias textArea: textArea
    readonly property alias copyMessage: copyMessage
    required property var document
    readonly property alias contentScroll: contentScroll
    property alias searchBar: searchBar 
    readonly property Kirigami.PageRow pageStack: (root.ApplicationWindow.window as Kirigami.ApplicationWindow)?.pageStack ?? null
    required property TextFieldContextMenu textFieldContextMenu

    property bool mobileToolBarHidden: true
    property real mobileToolBarHeight: 0

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
    rightPadding: 0
    topPadding: 0

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
                            if (text.length > 0) {
                                root.document.findText(text);
                            } else {
                                root.document.clearSearch();
                                textArea.deselect();
                            }
                        }
                        Keys.onShortcutOverride: (event) => event.accepted = (event.key === Qt.Key_Escape)
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
                        onClicked: {
                            searchBar.isReplaceVisible = !searchBar.isReplaceVisible
                            if (searchBar.isReplaceVisible) {
                                replaceField.forceActiveFocus()
                            }
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
                        Keys.onReturnPressed: {
                            if (event.modifiers & Qt.ControlModifier) {
                                document.replaceAll(replaceField.text);
                            } else {
                                document.replaceCurrent(replaceField.text);
                            }
                        }
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
        }
    }

    contentItem: ScrollView {
        id: contentScroll
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Layout.fillWidth: true
        Layout.fillHeight: true

        bottomPadding: root.isWideScreen || NavigationController.sourceMode ? 0 : (root.mobileToolBarHidden ? 0 : root.mobileToolBarHeight)
        // Animate scroll bar between wide and mobile screens transitions
        Behavior on bottomPadding {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutInQuart
            }
        }

        T.TextArea {
            id: textArea

            textMargin: root.isWideScreen? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 1
            leftPadding: 0
            rightPadding: 0
            topPadding: 0

            readonly property int additionalPadding: Kirigami.Units.gridUnit * 4

            bottomPadding: root.isWideScreen || NavigationController.sourceMode ? additionalPadding : (root.mobileToolBarHidden ? 0 : root.mobileToolBarHeight)

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

            property int lastKey: -1
            Keys.onPressed: (event) => {
                if (event.matches(StandardKey.Paste)) {
                    if (root.document && typeof root.document.pasteFromClipboard === 'function') {
                        root.document.pasteFromClipboard();
                        event.accepted = true;
                        return;
                    }
                }
                lastKey = event.key
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

            Shortcut {
                sequence: StandardKey.Find
                onActivated: if (searchBar.isSearchOpen === true) {
                    root.closeSearch()
                } else {
                    root.openSearch()
                }
            }

            Shortcut {
                sequence: StandardKey.Replace
                onActivated:
                {
                    root.openSearch()
                    searchBar.isReplaceVisible = true
                    if (replaceField) {
                        replaceField.forceActiveFocus()
                    }
                }
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
            if (ApplicationWindow.window) {
                ApplicationWindow.window.currentDocument = root.document;
            }
        }
    }

    onVisibleChanged: {
        if (!ApplicationWindow.window) {
            return;
        }

        if (visible) {
            ApplicationWindow.window.currentDocument = root.document
        } else if ((ApplicationWindow.window as Main).currentDocument === root.document) {
            ApplicationWindow.window.currentDocument = null
        }
    }

    Component.onDestruction: {
        // if (!saved && noteFullPath.toString().length > 0 && root.document !== null) {
        //     root.document.saveAs(noteFullPath);
        // }
        // this doesn't need to be called here as saving the document is already being called in the source mode changed handler

        if (ApplicationWindow.window !== null && (ApplicationWindow.window as Main).currentDocument === root.document) {
            ApplicationWindow.window.currentDocument = null
        }
    }

    onNoteFullPathChanged: () => {
        if (!init) {
            return;
        }
        loadNote();
    }
}
