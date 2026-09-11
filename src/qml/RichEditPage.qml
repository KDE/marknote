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

    canFitToolbar: width >= toolBar.width + Kirigami.Units.largeSpacing * 2

    mobileToolBarHidden: mobileToolBarContainer.hidden
    mobileToolBarHeight: mobileToolBarContainer.height

    supportsToc: true
    isTocOpened: tocDrawer.opened
    tocDrawer: tocDrawer

    objectName: "RichEditPage"

    contentComponent: BlockView {
        id: blockView
        anchors.fill: parent
        richDocumentHandler: richdochandler
    }

    document: RichDocumentHandler {
        id: richdochandler

        onError: message => {
            console.error("Error message from document handler", message);
        }


        Component.onCompleted: {
            CommandManager.setModel(richdochandler.treeModel);
        }

        onTreeModelChanged: {
            CommandManager.setModel(richdochandler.treeModel);
        }
    }

    Timer {
        id: saveTimer
        repeat: false
        interval: 1000
        onTriggered: {
            if (root.activeTextArea && typeof root.activeTextArea.flushTimer === "function") {
                root.activeTextArea.flushTimer();
            }
            if (root.noteFullPath.toString().length > 0) {
                root.document.saveAs(root.noteFullPath);
                root.saved = true;
            }
        }
    }

    Connections {
        target: CommandManager
        function onCanUndoChanged() {
            root.saved = false;
            saveTimer.restart();
        }
        function onInternalLinkActivated(noteName) {
            root.openNoteByName(noteName);
        }
    }

    Component.onDestruction: {
        if (root.activeTextArea && typeof root.activeTextArea.flushTimer === "function") {
            root.activeTextArea.flushTimer();
        }
        if (!root.saved && root.noteFullPath.toString().length > 0) {
            root.document.saveAs(root.noteFullPath);
            root.saved = true;
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

    property var insertTargetBlock: null
    property int insertTargetCursorPos: -1
    property int insertTargetSelectionStart: -1
    property int insertTargetSelectionEnd: -1
    property string insertTargetSelectedText: ""

    function saveInsertTarget() {
        insertTargetBlock = richdochandler.treeModel ? richdochandler.treeModel.focusedBlock() : null;
        if (activeTextArea) {
            insertTargetCursorPos = activeTextArea.cursorPosition;
            insertTargetSelectionStart = activeTextArea.selectionStart;
            insertTargetSelectionEnd = activeTextArea.selectionEnd;
            insertTargetSelectedText = activeTextArea.selectedText;
        } else {
            insertTargetCursorPos = -1;
            insertTargetSelectionStart = -1;
            insertTargetSelectionEnd = -1;
            insertTargetSelectedText = "";
        }
    }

    function restoreInsertTargetAndInsert(markdownText) {
        if (insertTargetBlock && richdochandler.treeModel) {
            richdochandler.treeModel.requestFocus(insertTargetBlock, insertTargetCursorPos);
            Qt.callLater(() => {
                if (activeTextArea) {
                    if (insertTargetSelectionStart !== insertTargetSelectionEnd) {
                        activeTextArea.remove(insertTargetSelectionStart, insertTargetSelectionEnd);
                    }
                    activeTextArea.insert(activeTextArea.cursorPosition, markdownText);
                }
            });
        }
    }

    LinkDialog {
        id: linkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: {
            let markdownLink = `[${linkText}](${linkUrl})`;
            restoreInsertTargetAndInsert(markdownLink);
        }
    }

    NoteLinkDialog {
        id: noteLinkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: {
            let alias = noteAlias ? noteAlias : noteName;
            let markdownLink = `[${alias}](marknote://note/${noteName})`;
            restoreInsertTargetAndInsert(markdownLink);
        }
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
                let markdownImage = `![](${fileUrl})`;
                restoreInsertTargetAndInsert(markdownImage);
            }
        }
    }

    TableDialog {
        id: tableDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: root.overlay
        onAccepted: {
            let tableMd = "\n";
            for (let r = 0; r < rows + 2; r++) {
                tableMd += "|";
                for (let c = 0; c < cols; c++) {
                    if (r === 1) tableMd += "---|";
                    else tableMd += "   |";
                }
                tableMd += "\n";
            }
            restoreInsertTargetAndInsert(tableMd);
        }
    }

    SketchDialog {
        id: sketchDialog
        notePath: root.noteFullPath

        onSaved: imagePath => {
            if (imagePath.toString().length > 0) {
                let markdownImage = `![](file://${imagePath})`;
                restoreInsertTargetAndInsert(markdownImage);
            }
        }
    }

    TocDrawer {
        id: tocDrawer

        treeModel: root.document.treeModel
        blockView: root.mainContentItem ? root.mainContentItem.listView : null

        topMargin: (root.pageStack && root.pageStack.globalToolBar) ? root.pageStack.globalToolBar.height : (root.ApplicationWindow.window && root.ApplicationWindow.window.header ? root.ApplicationWindow.window.header.height : 0)
        y: topMargin
        bottomMargin: 0

        height: parent ? parent.height - topMargin : 0
    }

    EmojierPopup {
        id: emojierPopup
        parent: root.Overlay.overlay
        filterText: root.document.currentEmojicode

        x: {
            if (!root.activeTextArea || !parent) return 0;

            let cursorRect = root.activeTextArea.positionToRectangle(root.activeTextArea.cursorPosition);
            let mappedPos = root.activeTextArea.mapToItem(parent, cursorRect.x, cursorRect.y);

            let targetX = mappedPos.x + 5;

            let minX = 0;
            let maxX = parent.width - width;

            return Math.max(minX, Math.min(targetX, maxX));
        }

        y: {
            if (!root.activeTextArea || !parent) return 0;

            let cursorRect = root.activeTextArea.positionToRectangle(root.activeTextArea.cursorPosition);
            let mappedPos = root.activeTextArea.mapToItem(parent, cursorRect.x, cursorRect.y);

            let targetY = mappedPos.y + Config.editorFont.pixelSize * 2;

            let minY = 0;
            let maxY = parent.height - height;

            if (targetY > maxY) {
                let aboveY = mappedPos.y;
                return Math.max(minY, aboveY - height - Config.editorFont.pixelSize);
            }

            return Math.max(minY, Math.min(targetY, maxY));
        }

        Connections {
            target: root.document
            function onPopupVisibleChanged(): void {
                if (emojierPopup.visibleItemsCount === 0) {
                    root.document.popupVisible = false;
                    return;
                }

                if (root.document.popupVisible) {
                    emojierPopup.open();
                } else {
                    emojierPopup.close();
                }
            }
        }

        onOpened: {
            EditorActions.activePopup = emojierPopup;
        }

        onClosed: {
            if (EditorActions.activePopup === emojierPopup) {
                EditorActions.activePopup = null;
            }
            root.document.popupVisible = false;
        }

        onEmojiSelected: (emojichar) => {
            if (!root.activeTextArea) return;
            let charsToReplace = root.document.currentEmojicode.length + 1;
            let pos = root.activeTextArea.cursorPosition;
            let text = root.activeTextArea.text;
            let before = text.substring(0, pos - charsToReplace);
            let after = text.substring(pos);
            root.activeTextArea.text = before + emojichar + after;
            root.activeTextArea.cursorPosition = before.length + emojichar.length;
            root.document.popupVisible = false;
            root.document.currentEmojicode = "";
        }
    }

    SlashMenuPopup {
        id: slashPopup
        parent: root.Overlay.overlay

        x: {
            if (!root.activeTextArea || !parent) return 0;

            let cursorRect = root.activeTextArea.positionToRectangle(root.activeTextArea.cursorPosition);
            let mappedPos = root.activeTextArea.mapToItem(parent, cursorRect.x, cursorRect.y);

            let targetX = mappedPos.x + 5;

            let minX = 0;
            let maxX = parent.width - width;

            return Math.max(minX, Math.min(targetX, maxX));
        }

        y: {
            if (!root.activeTextArea || !parent) return 0;

            let cursorRect = root.activeTextArea.positionToRectangle(root.activeTextArea.cursorPosition);
            let mappedPos = root.activeTextArea.mapToItem(parent, cursorRect.x, cursorRect.y);

            let targetY = mappedPos.y + Config.editorFont.pixelSize * 2;

            let minY = 0;
            let maxY = parent.height - height;

            if (targetY > maxY) {
                let aboveY = mappedPos.y;
                return Math.max(minY, aboveY - height - Config.editorFont.pixelSize);
            }

            return Math.max(minY, Math.min(targetY, maxY));
        }

        onOpened: {
            EditorActions.activePopup = slashPopup;
        }

        onClosed: {
            if (EditorActions.activePopup === slashPopup) {
                EditorActions.activePopup = null;
            }
        }

        onElementSelected: (item) => {
            root.handleSlashElementSelected(item);
        }
    }

    function checkForSlashCommand(text, cursorPosition) {
        if (!root.activeTextArea || !text || cursorPosition <= 0) {
            slashPopup.close();
            return;
        }

        let focused = CommandManager.model ? CommandManager.model.focusedBlock() : null;
        if (!focused || focused.type === MDOptions.ElementType.Code) {
            slashPopup.close();
            return;
        }

        let leftText = text.substring(0, cursorPosition);
        let lineStart = leftText.lastIndexOf('\n');
        let currentLineText = lineStart === -1 ? leftText : leftText.substring(lineStart + 1);

        let lastSlash = currentLineText.lastIndexOf('/');
        if (lastSlash === -1) {
            slashPopup.close();
            return;
        }

        if (lastSlash > 0 && currentLineText[lastSlash - 1] !== ' ' && currentLineText[lastSlash - 1] !== '\t') {
            slashPopup.close();
            return;
        }

        let query = currentLineText.substring(lastSlash + 1);
        let absoluteSlashPos = (lineStart === -1 ? 0 : lineStart + 1) + lastSlash;

        slashPopup.slashPosition = absoluteSlashPos;
        slashPopup.filterText = query;

        if (!slashPopup.opened) {
            slashPopup.open();
        }
    }

    function handleSlashElementSelected(item) {
        if (!item || item.type === "close") {
            slashPopup.close();
            return;
        }

        if (!root.activeTextArea) {
            slashPopup.close();
            return;
        }

        let currentBlock = CommandManager.model ? CommandManager.model.focusedBlock() : null;
        if (!currentBlock) {
            slashPopup.close();
            return;
        }

        let text = root.activeTextArea.text;
        let pos = root.activeTextArea.cursorPosition;
        let slashPos = slashPopup.slashPosition;

        let before = text.substring(0, slashPos);
        let after = text.substring(pos);
        let cleanText = before + after;

        root.activeTextArea.text = cleanText;
        root.activeTextArea.cursorPosition = slashPos;
        CommandManager.editText(currentBlock, text, cleanText, slashPos, slashPos);

        slashPopup.close();

        CommandManager.insertParagraphBelow(currentBlock, "");

        let newBlock = CommandManager.model ? CommandManager.model.focusedBlock() : null;
        if (!newBlock) {
            return;
        }

        let model = richdochandler.treeModel;

        switch (item.type) {
        case "paragraph":
            break;
        case "heading1":
            model.setItemMD(newBlock, "# ");
            model.requestFocus(newBlock);
            break;
        case "heading2":
            model.setItemMD(newBlock, "## ");
            model.requestFocus(newBlock);
            break;
        case "heading3":
            model.setItemMD(newBlock, "### ");
            model.requestFocus(newBlock);
            break;
        case "heading4":
            model.setItemMD(newBlock, "#### ");
            model.requestFocus(newBlock);
            break;
        case "heading5":
            model.setItemMD(newBlock, "##### ");
            model.requestFocus(newBlock);
            break;
        case "heading6":
            model.setItemMD(newBlock, "###### ");
            model.requestFocus(newBlock);
            break;
        case "divider":
            CommandManager.parseBlock(newBlock, "---");
            break;
        case "blockquote":
            CommandManager.transformToBlockquote(newBlock, 1, "");
            break;
        case "code":
            CommandManager.parseBlock(newBlock, "```\n\n```");
            break;
        case "unordered_list":
            CommandManager.transformToList(newBlock, false, 0, "");
            break;
        case "ordered_list":
            CommandManager.transformToList(newBlock, true, 1, "");
            break;
        case "task_list":
            CommandManager.transformToChecklist(newBlock, false, "");
            break;
        case "table":
            CommandManager.parseBlock(newBlock, "|  |  |  |\n| --- | --- | --- |\n|  |  |  |");
            break;
        }
    }

    Connections {
        target: root.activeTextArea
        function onCursorPositionChanged(): void {
            if (root.activeTextArea) {
                root.document.checkForShortcode(root.activeTextArea.text, root.activeTextArea.cursorPosition);
                root.checkForSlashCommand(root.activeTextArea.text, root.activeTextArea.cursorPosition);
            }
        }
        function onTextChanged(): void {
            root.saved = false;
            saveTimer.restart();
            if (root.activeTextArea) {
                root.document.checkForShortcode(root.activeTextArea.text, root.activeTextArea.cursorPosition);
                root.checkForSlashCommand(root.activeTextArea.text, root.activeTextArea.cursorPosition);
            }
        }
    }

    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Up"
        onActivated: emojierPopup.moveSelectionUp()
    }
    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Down"
        onActivated: emojierPopup.moveSelectionDown()
    }
    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Return"
        onActivated: emojierPopup.selectCurrent()
    }
    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Enter"
        onActivated: emojierPopup.selectCurrent()
    }
    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Tab"
        onActivated: emojierPopup.selectCurrent()
    }
    Shortcut {
        enabled: emojierPopup.opened
        sequence: "Escape"
        onActivated: emojierPopup.close()
    }
    Shortcut {
        sequence: "Ctrl+Space"
        onActivated: {
            if (root.activeTextArea) {
                root.document.checkForShortcode(root.activeTextArea.text, root.activeTextArea.cursorPosition);
            }
        }
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
                enabled: activeTextArea !== null

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
                enabled: activeTextArea !== null

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
                enabled: activeTextArea !== null

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
            if (richdochandler.treeModel && richdochandler.treeModel.focusedBlock()) {
                CommandManager.indentListItem(richdochandler.treeModel.focusedBlock(), richdochandler.treeModel.focusedBlockCursorPos());
            }
        }

        enabled: activeTextArea && CommandManager.canIndentListItem(richdochandler.treeModel.focusedBlock())
    }

    Kirigami.Action {
        id: dedentAction
        icon.name: "format-indent-less"
        text: KI18n.i18nc("@action:button", "Decrease List Level")
        onTriggered: {
            if (richdochandler.treeModel && richdochandler.treeModel.focusedBlock()) {
                CommandManager.deIndentListItem(richdochandler.treeModel.focusedBlock(), richdochandler.treeModel.focusedBlockCursorPos());
            }
        }

        enabled: activeTextArea && CommandManager.canDeIndentListItem(richdochandler.treeModel.focusedBlock())
    }

    Component {
        id: listFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                action: indentAction
                display: AbstractButton.IconOnly
                focusPolicy: Qt.NoFocus
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                action: dedentAction
                display: AbstractButton.IconOnly
                focusPolicy: Qt.NoFocus
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
    Component{
        id: listStyleGroup
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            enabled: activeTextArea !== null

            ToolButton {
                icon.name: "format-list-unordered"
                text: KI18n.i18nc("@action:button", "Unordered list")
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                focusPolicy: Qt.NoFocus
                onClicked: {
                    let block = richdochandler.treeModel.focusedBlock();
                    if (!block) return;
                    if (CommandManager.isListItem(block)) {
                        CommandManager.changeListType(block, 1);
                    } else {
                        CommandManager.transformToList(block, false, 1, activeTextArea.text);
                    }
                }
            }

            ToolButton {
                icon.name: "format-list-ordered"
                text: KI18n.i18nc("@action:button", "Ordered list")
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                focusPolicy: Qt.NoFocus
                onClicked: {
                    let block = richdochandler.treeModel.focusedBlock();
                    if (!block) return;
                    if (CommandManager.isListItem(block)) {
                        CommandManager.changeListType(block, 0);
                    } else {
                        CommandManager.transformToList(block, true, 1, activeTextArea.text);
                    }
                }
            }

            ToolButton {
                icon.name: "view-list-details"
                text: KI18n.i18nc("@action:button", "Checklist")
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                focusPolicy: Qt.NoFocus
                onClicked: {
                    let block = richdochandler.treeModel.focusedBlock();
                    if (!block) return;
                    if (CommandManager.isListItem(block)) {
                        CommandManager.changeListType(block, 2);
                    } else {
                        CommandManager.transformToList(block, false, 1, activeTextArea.text);
                        CommandManager.transformToChecklist(block, false, activeTextArea.text);
                    }
                }
            }
        }
    }
    Component{
        id: insertGroup

        RowLayout {
            ToolButton {
                id: linkAction
                icon.name: "insert-link-symbolic"
                text: KI18n.i18nc("@action:button", "Insert link")
                display: AbstractButton.IconOnly
                focusPolicy: Qt.NoFocus
                enabled: !!activeTextArea
                onClicked: {
                    saveInsertTarget();
                    linkDialog.linkText = insertTargetSelectedText;
                    linkDialog.linkUrl = "";
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
                focusPolicy: Qt.NoFocus
                enabled: !!activeTextArea
                onClicked: {
                    saveInsertTarget();
                    noteLinkDialog.noteAlias = insertTargetSelectedText;
                    noteLinkDialog.noteName = "";
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
                focusPolicy: Qt.NoFocus
                enabled: !!activeTextArea
                onClicked: {
                    saveInsertTarget();
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
                focusPolicy: Qt.NoFocus
                enabled: !!activeTextArea
                onClicked: {
                    saveInsertTarget();
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
                focusPolicy: Qt.NoFocus
                enabled: !!activeTextArea
                onClicked: {
                    saveInsertTarget();
                    sketchDialog.open();
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }

    function setHeadingLevel(level: int) {
        if (!activeTextArea) {
            return;
        }

        let str = activeTextArea["text"];

        const match = str.match(/^( *)#{0,6}( ?)/);
        const spaces = match[1];
        const content = str.slice(match[0].length);

        activeTextArea["text"] = spaces + (level ? '#'.repeat(level) + ' ' : '') + content;
    }

    property var currentHeadingLevel: {
        if (!activeTextArea) {
            return 0;
        }

        let str = activeTextArea["text"];

        let i = 0;
        while (i < str.length && str[i] === ' ') {
            i++;
        }

        let level = 0;
        while (i < str.length && str[i] === '#' && level < 6) {
            level++;
            i++;
        }

        return level;
    }

    Component {
        id: headingGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            enabled: activeTextArea !== null

            ToolButton {
                text: "¶"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 0
                onClicked: setHeadingLevel(0)
                ToolTip.text: KI18n.i18nc("@item:inmenu no heading", "Paragraph")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H1"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 1
                onClicked: setHeadingLevel(1)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 1 (largest)", "Heading 1")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H2"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 2
                onClicked: setHeadingLevel(2)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 2", "Heading 2")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H3"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 3
                onClicked: setHeadingLevel(3)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 3", "Heading 3")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H4"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 4
                onClicked: setHeadingLevel(4)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 4", "Heading 4")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H5"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 5
                onClicked: setHeadingLevel(5)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 5", "Heading 5")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                text: "H6"
                display: AbstractButton.TextOnly
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: currentHeadingLevel === 6
                onClicked: setHeadingLevel(6)
                ToolTip.text: KI18n.i18nc("@item:inmenu heading level 6 (smallest)", "Heading 6")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
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

                    RowLayout {
                        id: categorySelector

                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.topMargin: 0
                        Layout.fillWidth: true
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                        Layout.alignment: Qt.AlignHCenter
                        spacing: Kirigami.Units.smallSpacing

                        property int selectedIndex: 0

                        ToolButton {
                            text: KI18n.i18n("Format")
                            focusPolicy: Qt.NoFocus
                            checkable: true
                            autoExclusive: true
                            checked: categorySelector.selectedIndex === 0
                            onClicked: categorySelector.selectedIndex = 0
                            Layout.fillWidth: true
                        }
                        ToolButton {
                            text: KI18n.i18n("Lists")
                            focusPolicy: Qt.NoFocus
                            checkable: true
                            autoExclusive: true
                            checked: categorySelector.selectedIndex === 1
                            onClicked: categorySelector.selectedIndex = 1
                            Layout.fillWidth: true
                        }
                        ToolButton {
                            text: KI18n.i18n("Insert")
                            focusPolicy: Qt.NoFocus
                            checkable: true
                            autoExclusive: true
                            checked: categorySelector.selectedIndex === 2
                            onClicked: categorySelector.selectedIndex = 2
                            Layout.fillWidth: true
                        }
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
}
