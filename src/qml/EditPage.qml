// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
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


    readonly property bool wideScreen: width >= toolBar.width + Kirigami.Units.largeSpacing * 2

    required property var document
    property bool init: false

    property bool saved: true
    property string oldPath: ''
    property bool listIndent: true
    property bool listDedent: true
    property bool checkbox: false
    property int listStyle
    property int heading
    property bool singleDocumentMode: false

    // Only overwrite these values in MainEditor
    property string noteName: NavigationController.noteName
    property string noteFullPath: NavigationController.noteFullPath
    property alias headerItems: headerItemsRow.children
    readonly property alias textArea: textArea
    readonly property alias contentScroll: contentScroll
    property alias searchBar: searchBar 

    objectName: "EditPage"

    leftPadding: 0
    rightPadding: Kirigami.Settings.isMobile ? 0 : (tocDrawer.width * tocDrawer.position)
    topPadding: 0
    bottomPadding: 0

    Layout.fillWidth: true

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
        if (!ApplicationWindow.window) {
            return;
        }

        if (visible) {
            ApplicationWindow.window.currentDocument = document
        } else if (ApplicationWindow.window.currentDocument === document) {
            ApplicationWindow.window.currentDocument = null
        }
    }

    Component.onDestruction: {
        if (ApplicationWindow.window !== null && ApplicationWindow.window.currentDocument === document) {
            ApplicationWindow.window.currentDocument = null
        }
    }

    onNoteFullPathChanged: () => {
        if (!init) {
            return;
        }
        loadNote();
    }

    titleDelegate: RowLayout {
        id: headerItemsRow

        visible: root.noteName

        Layout.fillWidth: true
    }

    Component{
        id: richDocumentHandler
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
        id: contentScroll
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Layout.fillWidth: true
        Layout.fillHeight: true

        bottomPadding: wideScreen || NavigationController.sourceMode ? 0 : (mobileToolBarHidden ? 0 : mobileToolBarHeight)
        // Animate scroll bar between wide and mobile screens transitions
        Behavior on bottomPadding {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutInQuart
            }
        }

        T.TextArea {
            id: textArea

            readonly property int additionalPadding: Kirigami.Units.gridUnit * 4

            textMargin: wideScreen? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 1

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: wideScreen || NavigationController.sourceMode ? additionalPadding : (mobileToolBarHidden ? 0 : mobileToolBarHeight)

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
                    document.slotMouseMovedWithControl(controlHoverHandler.point.position)
                }

                onHoveredChanged: () => {
                    if (!controlHoverHandler.hovered) {
                        document.slotMouseMovedWithControlReleased()
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

            onPressAndHold: {
                if (Kirigami.Settings.tabletMode && selectByMouse) {
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }

            property int lastKey: -1
            Keys.onPressed: (event) => {
                if (event.matches(StandardKey.Paste)) {
                    if (document && typeof document.pasteFromClipboard === 'function') {
                        document.pasteFromClipboard();
                        event.accepted = true;
                        return;
                    }
                }
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
                        if (ApplicationWindow.window !== null && ApplicationWindow.window.currentDocument === document) {
                            ApplicationWindow.window.currentDocument = null;
                        }
                    }

                    Component.onDestruction: {
                        if (!saved && root.noteFullPath.toString().length > 0) {
                            document.saveAs(root.noteFullPath);
                        }
                        // Safely check if the window still exists first!
                        if (ApplicationWindow.window !== null && ApplicationWindow.window.currentDocument === document) {
                            ApplicationWindow.window.currentDocument = null;
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
