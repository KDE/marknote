// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick.Templates as T
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.marknote
import org.kde.syntaxhighlighting

EditPage {
    id: root

    objectName: "RawEditPage"
    
    property T.TextArea textArea: null

    contentComponent: ScrollView {
        id: contentScroll
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Layout.fillWidth: true
        Layout.fillHeight: true

        bottomPadding: root.canFitToolbar || root.mobileToolBarHidden ? 0 : root.mobileToolBarHeight

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
            readonly property bool applyWideScreenMargin: (root.isWideScreen && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns) || root.singleDocumentMode
            textMargin: applyWideScreenMargin ? Kirigami.Units.largeSpacing * marginMultiplier : Kirigami.Units.smallSpacing * marginMultiplier

            leftPadding: 0
            rightPadding: Kirigami.Units.largeSpacing
            topPadding: 0
            bottomPadding: 0

            Component.onCompleted: {
                root.textArea = textArea
            }

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
            textFormat: TextEdit.PlainText
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
                    root.toggleSearch();
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

    document: RawDocumentHandler {
        cursorPosition: root.textArea.cursorPosition
        document: root.textArea.textDocument
        selectionStart: root.textArea.selectionStart
        selectionEnd: root.textArea.selectionEnd
        textArea: root.textArea

        onCopy: root.textArea.copy()

        onCut: root.textArea.cut()
        onError: message => {
            console.error("Error message from document handler", message);
        }
        // textColor: TODO
        onLoaded: text => {
            root.textArea.text = text;
        }
        onMoveCursor: position => {
            root.textArea.cursorPosition = position;
        }
        onRedo: root.textArea.redo()
        onSelectCursor: (start, end) => {
            root.textArea.select(start, end);
        }
        onUndo: root.textArea.undo()
    }

    SyntaxHighlighter {
        textEdit: root.textArea
        definition: "Markdown"
    }

    textFieldContextMenu: TextFieldContextMenu {
        document: root.document
    }
}