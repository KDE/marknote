// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

import org.kde.kirigami as Kirigami
import org.kde.marknote
import org.kde.ki18n
import org.kde.kirigamiaddons.components as Components

EditPage {
    id: root

    property bool listIndent: true
    property bool listDedent: true
    property bool checkbox: false
    property int listStyle: 0
    property int heading: 0

    canFitToolbar: width >= toolBar.width + Kirigami.Units.largeSpacing * 2

    mobileToolBarHidden: mobileToolBarContainer.hidden
    mobileToolBarHeight: mobileToolBarContainer.height

    dynamicRightPadding: tocDrawer.position * tocDrawer.width

    supportsToc: true
    isTocOpened: tocDrawer.opened
    tocPosition: tocDrawer.position
    tocDrawer: tocDrawer

    function toggleToc() {
        if (tocDrawer.opened) {
            tocDrawer.close()
        } else {
            tocDrawer.open()
        }
    }

    objectName: "RichEditPage"

    contentComponent: BlockView {
        id: blockView
        anchors.fill: parent
        richDocumentHandler: richdochandler
    }

    document: RichDocumentHandler {
        id: richdochandler

        blockMargin: Kirigami.Units.largeSpacing

        onError: message => {
            console.error("Error message from document handler", message);
        }

        onCheckableChanged: {
            root.checkbox = checkable;
        }

        Component.onCompleted: {
            CommandManager.setModel(richdochandler.treeModel);
        }

        onTreeModelChanged: {
            CommandManager.setModel(richdochandler.treeModel);
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

    LinkDialog {
        id: linkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: root.document.updateLink(linkUrl, linkText)
    }

    NoteLinkDialog {
        id: noteLinkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: root.document.updateNoteLink(noteName, noteAlias)
    }

    FileDialog {
        id: imageDialog

        title: KI18n.i18nc("@title:window", "Select an image")
        currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        fileMode: FileDialog.OpenFile
        nameFilters: [KI18n.i18n("Image files (*.jpg *.jpeg *.jxl *.png *.svg *.webp)"), KI18n.i18n("All files (*)")]

        onAccepted: {
            const fileUrl = selectedFile.toString();
            if (fileUrl.length > 0) {
                root.document.insertImage(fileUrl);
            }
        }
    }

    TableDialog {
        id: tableDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: root.document.insertTable(rows, cols)
    }

    SketchDialog {
        id: sketchDialog
        notePath: root.noteFullPath

        onSaved: imagePath => {
            if (imagePath.toString().length > 0) {
                root.document.insertImage("file://" + imagePath);
            }
        }
    }

    TocDrawer {
        id: tocDrawer

        treeModel: root.document.treeModel
        blockView: root.mainContentItem ? root.mainContentItem.listView : null
        parent: root.overlay

        topMargin: (root.pageStack && root.pageStack.globalToolBar) ? root.pageStack.globalToolBar.height : (root.ApplicationWindow.window && root.ApplicationWindow.window.header ? root.ApplicationWindow.window.header.height : 0)
        bottomMargin: 0

        height: parent.height - topMargin
    }

    property Item activeTextArea: {
        const window = root.Window.window
        if (!window) {
            return null;
        }

        const textArea = window.activeFocusItem
        if (!textArea || !(textArea instanceof TextArea)) {
            return null;
        }

        return textArea;
    }

    property var textAreaSpans: {
        if (!activeTextArea) {
            return [];
        }

        const txt = activeTextArea["text"];
        if (txt == "") {
            return [];
        }

        return MDInlineStyleParser.parse(txt);
    }

    property var currentSpan: {
        if (!activeTextArea) {
            return {start: -1, end: -1, type: MDOptions.InlineStyle.None};
        }

        let cursorPos = activeTextArea["cursorPosition"];
        let obj = getCurrentWord(activeTextArea["text"], cursorPos);

        for (let span of textAreaSpans) {
            if (span.start <= cursorPos && cursorPos <= span.end) {
                return span;
            }
        }

        return {start: -1, end: -1, type: MDOptions.InlineStyle.None};
    }

    function getCurrentWord(txt, cursorPosition) {
        const leftSpace = Math.max(0, txt.lastIndexOf(" ", cursorPosition));
        const leftNewline = Math.max(0, txt.lastIndexOf("\n", cursorPosition));

        let rightSpace = txt.indexOf(" ", cursorPosition);
        if (rightSpace === -1) {
            rightSpace = txt.length - 1
        }

        let rightNewline = txt.indexOf("\n", cursorPosition);
        if (rightNewline === -1) {
            rightNewline = txt.length - 1
        }

        return {left: Math.max(leftSpace, leftNewline), right: Math.min(rightSpace, rightNewline)};
    }

    function removeCurrentStyle() {
        if (!activeTextArea || currentSpan.type == MDOptions.InlineStyle.None) {
            return;
        }

        let cursorPos = activeTextArea["cursorPosition"];

        let txt = activeTextArea["text"];
        let first = currentSpan.start;
        let second = currentSpan.start + 1;
        let third = currentSpan.end;
        let fourth = currentSpan.end + 1;

        if (currentSpan.type == MDOptions.InlineStyle.Strong) {
            second++;
            third--;
            cursorPos--;
        }

        activeTextArea["text"] = txt.slice(0, first) + txt.slice(second, third) + txt.slice(fourth);
        activeTextArea["cursorPosition"] = cursorPos - 1;
    }

    function getMarkerForStyle(styleType) {
        if (styleType === MDOptions.InlineStyle.Emphasis) return "*";
        if (styleType === MDOptions.InlineStyle.Strong) return "**";
        if (styleType === MDOptions.InlineStyle.Strikethrough) return "~";
        return "";
    }

    function addInlineStyle(styleType) {
        if (!activeTextArea) return;

        const marker = getMarkerForStyle(styleType);
        if (!marker) return;

        const txt = activeTextArea.text;
        const selStart = activeTextArea.selectionStart;
        const selEnd = activeTextArea.selectionEnd;

        if (selStart !== selEnd) {
            activeTextArea.text = txt.slice(0, selStart) + marker + txt.slice(selStart, selEnd) + marker + txt.slice(selEnd);
            activeTextArea.select(selStart + marker.length, selEnd + marker.length);
        } else {
            let cursorPos = activeTextArea.cursorPosition;
            let bounds = getCurrentWord(txt, cursorPos);
            
            let left = bounds.left;
            if (txt[left] === ' ' || txt[left] === '\n') {
                left += 1;
            }
            let right = bounds.right;
            if (txt[right] === ' ' || txt[right] === '\n') {
                right -= 1;
            }
            
            if (left > right) {
                // Empty word (cursor on space or empty text)
                activeTextArea.text = txt.slice(0, cursorPos) + marker + marker + txt.slice(cursorPos);
                activeTextArea.cursorPosition = cursorPos + marker.length;
                return;
            }
            
            activeTextArea.text = txt.slice(0, left) + marker + txt.slice(left, right + 1) + marker + txt.slice(right + 1);
            activeTextArea.cursorPosition = cursorPos + marker.length;
        }
    }

    function toggleInlineStyle(styleType) {
        if (currentSpan.type === styleType) {
            removeCurrentStyle();
        } else {
            if (currentSpan.type !== MDOptions.InlineStyle.None) {
                removeCurrentStyle();
            }
            addInlineStyle(styleType);
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
                focusPolicy: Qt.NoFocus

                checked: currentSpan.type == MDOptions.InlineStyle.Strong

                onClicked: toggleInlineStyle(MDOptions.InlineStyle.Strong)

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
                focusPolicy: Qt.NoFocus

                checked: currentSpan.type == MDOptions.InlineStyle.Emphasis

                onClicked: toggleInlineStyle(MDOptions.InlineStyle.Emphasis)

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                icon.name: "format-text-strikethrough"
                text: KI18n.i18nc("@action:button", "Strikethrough")
                display: AbstractButton.IconOnly
                checkable: true
                focusPolicy: Qt.NoFocus

                checked: currentSpan.type == MDOptions.InlineStyle.Strikethrough

                onClicked: toggleInlineStyle(MDOptions.InlineStyle.Strikethrough)

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
                text: KI18n.i18nc("@action:button", "Insert note link")
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

            ToolButton {
                id: sketchAction
                icon.name: "draw-freehand"
                text: KI18n.i18nc("@action:button", "Insert sketch")
                display: AbstractButton.IconOnly

                onClicked: {
                    sketchDialog.open();
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
        id: floatingEditButton
        icon.name: "document-edit"
        parent: root.Overlay.overlay
        visible: !root.canFitToolbar && !NavigationController.sourceMode
        scale: mobileToolBarContainer.hidden ? 1.0 : 0.0

        property int defaultSpacing: Kirigami.Units.largeSpacing * 2

        Behavior on scale {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuart
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right

            rightMargin: defaultSpacing
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
                        id: undoButton
                        action: EditorActions.undoAction
                        display: AbstractButton.IconOnly
                        ToolTip.text: action.text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    ToolButton {
                        action: EditorActions.redoAction
                        display: AbstractButton.IconOnly
                        ToolTip.text: action.text
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

                    Item {
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
}
