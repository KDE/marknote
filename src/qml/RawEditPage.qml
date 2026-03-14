// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.marknote
import org.kde.ki18n
import org.kde.syntaxhighlighting

EditPage {
    id: root

    objectName: "RawEditPage"

    document: RawDocumentHandler {
        cursorPosition: root.textArea.cursorPosition
        document: root.textArea.textDocument
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

    titleDelegate: RowLayout {
        visible: root.noteName
        Layout.fillWidth: true
        ToolButton {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: root.textArea.canUndo
            icon.name: "edit-undo"
            text: KI18n.i18n("Undo")
            visible: root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            onClicked: root.textArea.undo()
        }
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: root.textArea.canRedo
            icon.name: "edit-redo"
            text: KI18n.i18n("Redo")
            visible: root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            onClicked: root.textArea.redo()
        }

        ToolButton {
            icon.name: "search"
            text: KI18n.i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: !root.isWideScreen && root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn
            checkable: true
            checked: root.searchBar.isSearchOpen
            onClicked: if (root.searchBar.isSearchOpen === true) {
                root.closeSearch();
            } else {
                root.openSearch();
            }

            ToolTip.text: KI18n.i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Item {
            Layout.fillWidth: true
        }

        Rectangle {
            color: Kirigami.Theme.textColor
            Layout.preferredHeight: 5
            Layout.preferredWidth: 5
            radius: 5
            scale: root.saved ? 0 : 1

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
        }

        Kirigami.Heading {
            text: root.noteName
            elide: Text.ElideRight
            wrapMode: Text.NoWrap

            Layout.rightMargin: Kirigami.Units.mediumSpacing
            Layout.leftMargin: Kirigami.Units.mediumSpacing
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth + Kirigami.Units.mediumSpacing * 2
            Layout.minimumWidth: 0
        }

        Item {
            Layout.fillWidth: true
        }

        Item {
            visible: root.isWideScreen
            Layout.preferredWidth: fillWindowButton.width
        }

        ToolButton {
            id: mobileUndoButton
            Layout.leftMargin: Kirigami.Units.smallSpacing
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: root.textArea.canUndo
            icon.name: "edit-undo"
            text: KI18n.i18n("Undo")
            visible: root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn

            onClicked: root.textArea.undo()
        }

        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: root.textArea.canRedo
            icon.name: "edit-redo"
            text: KI18n.i18n("Redo")
            visible: root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn

            onClicked: root.textArea.redo()
        }

        ToolButton {
            icon.name: "search"
            text: KI18n.i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: root.pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns
            checkable: true
            checked: root.searchBar.isSearchOpen
            onClicked: if(root.searchBar.isSearchOpen === true) {
                root.closeSearch()
            } else {
                root.openSearch()
            }

            ToolTip.text: KI18n.i18nc("@info:tooltip", "Search in Note")
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
            visible: root.isWideScreen || Config.fillWindow

            Behavior on columnWidth {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            onClicked: {
                Config.fillWindow = !Config.fillWindow;
            }
            onColumnWidthChanged: root.pageStack.defaultColumnWidth = columnWidth

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
            text: KI18n.i18nc("@action:menu", "Exit Full Screen")
            visible: root.Window.window.visibility === Window.FullScreen

            onClicked: root.Window.window.showNormal()
        }

        Button{
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: KI18n.i18n("Switch editor to preview mode")
            ToolTip.visible: hovered
            icon.name: "code-context-symbolic"
            checkable: true
            checked: true
            text: KI18n.i18n("Source View")
            padding: 0
            flat: true
            spacing: Kirigami.Units.smallSpacing

            display: AbstractButton.IconOnly

            onClicked: {
                document.saveAs(root.noteFullPath)
                NavigationController.sourceMode = !NavigationController.sourceMode
            }
        }
    }

    textFieldContextMenu: TextFieldContextMenu {
        document: root.document
    }
}
