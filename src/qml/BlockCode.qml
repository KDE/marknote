// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting
import org.kde.marknote

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.smallSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    readonly property var blockData: root.block.data

    readonly property var text: blockData.text
    property var language: blockData.lang

    property int fontSize: Config.editorFont.pointSize

    readonly property bool isMermaid: MermaidHelper.isMermaid(String(root.language || ""), String(root.text || ""))
    readonly property bool hasCode: String(root.text || "").trim().length > 0
    property bool sourceVisible: false
    readonly property bool showDiagram: root.isMermaid && !root.sourceVisible && root.hasCode
    property string diagramId: {
        // Bind to editor font and theme colors so font or palette changes re-request the SVG
        const _font = Config.editorFont;
        const _bg = Kirigami.Theme.backgroundColor;
        const _fg = Kirigami.Theme.textColor;
        const _accent = Kirigami.Theme.highlightColor;
        return root.showDiagram ? MermaidHelper.diagramId(String(root.text || ""), _bg.hslLightness < 0.5) : "";
    }

    onIsMermaidChanged: {
        if (root.isMermaid && !root.hasCode) {
            root.sourceVisible = true;
        }
    }

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: root.showDiagram
                        ? Math.max(diagramContainer.implicitHeight, buttonRow.implicitHeight + buttonRow.anchors.topMargin * 2)
                        : scrollView.implicitHeight

        Item {
            id: diagramContainer
            visible: root.showDiagram
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            implicitHeight: Math.max(Kirigami.Units.gridUnit * 6, diagramColumn.implicitHeight + Kirigami.Units.largeSpacing * 2)

            Rectangle {
                anchors.fill: parent
                color: Qt.alpha(Kirigami.Theme.textColor, 0.1)
                radius: Kirigami.Units.smallSpacing
            }

            ColumnLayout {
                id: diagramColumn
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.smallSpacing

                Image {
                    id: diagramImage
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 2
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    source: root.showDiagram && root.diagramId.length > 0 ? ("image://mermaid/" + root.diagramId) : ""
                }

                Kirigami.InlineMessage {
                    id: errorMessage
                    Layout.fillWidth: true
                    type: Kirigami.MessageType.Error
                    visible: diagramImage.status === Image.Error
                    text: i18n("Diagram rendering failed. Check Mermaid syntax.")
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: diagramImage.status === Image.Loading
                visible: running
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.sourceVisible = true;
                    if (root.cppModel) {
                        root.cppModel.requestFocus(root.block, codeText.text.length);
                    }
                    codeText.forceActiveFocus();
                }
            }
        }

        Flickable {
            id: scrollView

            visible: !root.showDiagram

            implicitWidth: parent.width
            implicitHeight: codeText.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)

            contentWidth: codeText.width
            contentHeight: codeText.implicitHeight
            
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            TextArea {
                id: codeText
                text: blockData.text
                font.family: Kirigami.Theme.fixedWidthFont.family
                font.pointSize: root.fontSize
                color: Kirigami.Theme.textColor
                padding: Kirigami.Units.largeSpacing

                width: Math.max(implicitWidth, scrollView.width)
                implicitHeight: Math.max(buttonRow.implicitHeight + buttonRow.anchors.topMargin * 2, contentHeight + topPadding + bottomPadding)

                background: Rectangle {
                    color: Qt.alpha(Kirigami.Theme.textColor, 0.1)
                    radius: Kirigami.Units.smallSpacing
                }
                selectionColor: if (NavigationController.notebookColor) {
                                    Kirigami.ColorUtils.tintWithAlpha(NavigationController.notebookColor, Kirigami.Theme.backgroundColor, 0.4)
                                } else {
                                    Kirigami.Theme.highlightColor
                                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    cursorShape: Qt.IBeamCursor
                    onClicked: (mouse) => {
                        root.openContextMenu(codeText, mouse.x, mouse.y);
                    }
                }

                property string lastSavedText: root.blockData.text
                property int lastSavedCursorPos: 0
                property bool isProgrammaticUpdate: false

                Timer {
                    id: editTimer
                    interval: 500
                    repeat: false
                    onTriggered: {
                        if (codeText.lastSavedText !== codeText.text || languageTextField.lastSavedLanguage !== languageTextField.text) {
                            CommandManager.editCode(root.block, codeText.lastSavedText, codeText.text, codeText.lastSavedCursorPos, codeText.cursorPosition, languageTextField.lastSavedLanguage, languageTextField.text);
                            codeText.lastSavedText = codeText.text;
                            codeText.lastSavedCursorPos = codeText.cursorPosition;
                            languageTextField.lastSavedLanguage = languageTextField.text;
                        }
                    }
                }

                function flushTimer() {
                    if (editTimer.running) {
                        editTimer.stop();
                        editTimer.triggered();
                    }
                }

                onTextChanged: {
                    if (isProgrammaticUpdate) return;
                    if (codeText.activeFocus && codeText.text !== codeText.lastSavedText) {
                        editTimer.restart();
                    }
                }

                Keys.onReturnPressed: event => {
                    if (EditorActions.handleKeyEvent(event)) {
                        return;
                    }

                    if (event.modifiers & Qt.ShiftModifier) {
                        editTimer.stop();
                        CommandManager.splitCode(root.block, codeText.lastSavedText, codeText.text, codeText.cursorPosition, languageTextField.lastSavedLanguage, languageTextField.text);
                        codeText.lastSavedText = codeText.text.substring(0, codeText.cursorPosition);
                        event.accepted = true;
                    } else {
                        flushTimer();
                        event.accepted = false;
                    }
                }

                Keys.onShortcutOverride: (event) => {
                    if (event.key === Qt.Key_Escape) {
                        event.accepted = true;
                    }
                }

                Keys.onPressed: (event) => {
                    if (EditorActions.handleKeyEvent(event)) {
                        return;
                    }

                    if (event.key === Qt.Key_Escape) {
                        if (root.isMermaid && root.hasCode) {
                            flushTimer();
                            root.sourceVisible = false;
                            event.accepted = true;
                            return;
                        }
                        if (root.cppModel) {
                            root.cppModel.clearFocus();
                            root.cppModel.clearSelection();
                        }
                        codeText.focus = false;
                        event.accepted = true;
                        return;
                    }

                    if (event.matches(StandardKey.SelectAll)) {
                        if (codeText.text.length === 0 || codeText.selectedText.length === codeText.text.length) {
                            if (root.cppModel) {
                                root.cppModel.clearFocus();
                                root.cppModel.selectAll();
                            }
                            codeText.focus = false;
                            event.accepted = true;
                            return;
                        }
                    }

                    if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                        flushTimer();
                        if (event.modifiers & Qt.ShiftModifier) {
                            CommandManager.redo();
                        } else {
                            CommandManager.undo();
                        }
                        event.accepted = true;
                    }

                    if (event.key === Qt.Key_Up) {
                        let textBeforeCursor = codeText.text.substring(0, codeText.cursorPosition);
                        if (textBeforeCursor.indexOf('\n') === -1) {
                            CommandManager.moveToPreviousBlock(root.block, codeText.text, codeText.cursorPosition);
                            flushTimer();
                            event.accepted = true;
                        }
                    }

                    if (event.key === Qt.Key_Down) {
                        let textAfterCursor = codeText.text.substring(codeText.cursorPosition);
                        if (textAfterCursor.indexOf('\n') === -1) {
                            CommandManager.moveToNextBlock(root.block, codeText.text, codeText.cursorPosition);
                            flushTimer();
                            event.accepted = true;
                        }
                    }
                }

                onActiveFocusChanged: {
                    if (!activeFocus) {
                        if (root.cppModel && root.cppModel.focusedBlock() === root.block) {
                            let activeItem = codeText.Window.activeFocusItem;
                            let isFallback = !activeItem;
                            let p = root.parent;
                            while (p && !isFallback) {
                                if (activeItem === p) isFallback = true;
                                p = p.parent;
                            }

                            if (!isFallback) {
                                root.cppModel.setFocusedBlock(null, -1);
                            } else {
                                // Focus lost due to some unknown reason
                                // We need to bring it back
                                Qt.callLater(() => {
                                    root.cppModel.requestFocus(root.block, root.cppModel.focusedBlockCursorPos());
                                })
                                return;
                            }
                        }
                        flushTimer();
                        if (root.isMermaid && root.hasCode) {
                            Qt.callLater(() => {
                                if (!root) return;
                                if (!codeText.activeFocus && !languageTextField.activeFocus) {
                                    root.sourceVisible = false;
                                }
                            });
                        }
                    } else {
                        codeText.lastSavedCursorPos = codeText.cursorPosition;
                        if (root.cppModel) {
                            root.cppModel.setFocusedBlock(root.block, codeText.cursorPosition);
                        }
                    }
                }

                Connections {
                    target: root.cppModel

                    function onFocusRequested(requestedBlock, cursorPosition) {
                        if (requestedBlock === root.block) {
                            if (root.showDiagram) {
                                root.sourceVisible = true;
                            }
                            if (cursorPosition >= 0) {
                                codeText.cursorPosition = cursorPosition;
                            }
                            codeText.forceActiveFocus();
                        } else {
                            if (root.isMermaid && root.hasCode) {
                                codeText.flushTimer();
                                root.sourceVisible = false;
                            }
                        }
                    }
                }

                Connections {
                    target: root

                    function onTextChanged() {
                        codeText.lastSavedText = root.text
                        if (codeText.text !== root.text) {
                            codeText.isProgrammaticUpdate = true;
                            codeText.text = root.text;
                            codeText.isProgrammaticUpdate = false;
                        }
                    }
                }

                Component.onCompleted: {
                    Qt.callLater(() => {
                        if (!root)
                            return;

                        if (root.cppModel && root.block === root.cppModel.focusedBlock()) {
                            codeText.cursorPosition = root.cppModel.focusedBlockCursorPos();
                            codeText.forceActiveFocus();
                        }
                    })
                }
            }

            Loader {
                active: root.language.length > 0
                        && (Repository.definitionForName(root.language).isValid ?? (Repository.definitionForName(root.language).section.length > 0))
                sourceComponent: SyntaxHighlighter {
                    textEdit: codeText
                    definition: root.language
                }
            }


            MouseArea {
                anchors.fill: codeText
                enabled: !codeText.activeFocus
                cursorShape: Qt.IBeamCursor
                hoverEnabled: true

                onClicked: (mouse) => {
                    if (root.cppModel) {
                        let clickIndex = codeText.positionAt(mouse.x, mouse.y);
                        root.cppModel.requestFocus(root.block, clickIndex);
                    }
                }
            }
        }

        RowLayout {
            id: buttonRow

            anchors {
                right: parent.right
                top: parent.top
                rightMargin: Kirigami.Units.mediumSpacing
                topMargin: Kirigami.Units.mediumSpacing
            }

            KQuickControlsAddons.Clipboard { id: clipboard }

            TextField {
                id: languageTextField

                text: root.blockData.lang
                placeholderText: "</>"
                color: Kirigami.Theme.disabledTextColor
                horizontalAlignment: Text.AlignHCenter

                TextMetrics {
                    id: textMetrics
                    font: languageTextField.font
                    text: languageTextField.text.length > 0
                          ? languageTextField.text
                          : languageTextField.placeholderText
                }

                Layout.maximumWidth: textMetrics.width + leftPadding + rightPadding + Kirigami.Units.smallSpacing

                background: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    radius: Kirigami.Units.cornerRadius
                    opacity: languageTextField.hovered || languageTextField.activeFocus ? 0.5 : 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuart
                        }
                    }
                }

                onTextChanged: {
                    root.language = text
                    if (codeText.isProgrammaticUpdate) return;
                    if (languageTextField.activeFocus && languageTextField.text !== languageTextField.lastSavedLanguage) {
                        editTimer.restart();
                    }
                }
                onActiveFocusChanged: {
                    if (!activeFocus && root.isMermaid && root.hasCode) {
                        Qt.callLater(() => {
                            if (!root) return;
                            if (!codeText.activeFocus && !languageTextField.activeFocus) {
                                root.sourceVisible = false;
                            }
                        });
                    }
                }
                property string lastSavedLanguage: root.blockData.lang
            }

            Button {
                icon.source: "edit-copy-symbolic"
                onClicked: {
                    clipboard.content = codeText.text
                    showPassiveNotification(i18n("Copied to clipboard!"))
                }
            }
        }
    }
}
