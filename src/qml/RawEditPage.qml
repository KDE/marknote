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

EditPage {
    id: root

    objectName: "RawEditPage"

    headerItems: [
        ToolButton {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canUndo
            icon.name: "edit-undo"
            text: i18n("Undo")
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            onClicked: textArea.undo()
        },
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canRedo
            icon.name: "edit-redo"
            text: i18n("Redo")
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            onClicked: textArea.redo()
        },
        ToolButton {
            icon.name: "search"
            text: i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: !wideScreen && pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn
            checkable: true
            checked: searchBar.isSearchOpen
            onClicked: {
                if(searchBar.isSearchOpen === true) {
                    closeSearch()
                } else {
                    openSearch()
                }
            }

            ToolTip.text: i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        },
        Item {
            width: Kirigami.Units.largeSpacing * 9
            visible: !wideScreen && pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn
        },
        Item {
            Layout.fillWidth: true
        },
        Rectangle {
            color: Kirigami.Theme.textColor
            height: 5
            radius: 2.5
            scale: root.saved ? 0 : 1
            width: height

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }
        },
        Kirigami.Heading {
            Layout.leftMargin: Kirigami.Units.mediumSpacing
            Layout.rightMargin: Kirigami.Units.mediumSpacing
            text: noteName
        },
        Item {
            Layout.fillWidth: true
        },
        Item {
            visible: wideScreen
            width: fillWindowButton.width
        },
        ToolButton {
            id: mobileUndoButton
            Layout.leftMargin: Kirigami.Units.smallSpacing
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canUndo
            icon.name: "edit-undo"
            text: i18n("Undo")
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn

            onClicked: textArea.undo()
        },
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            display: AbstractButton.IconOnly
            enabled: textArea.canRedo
            icon.name: "edit-redo"
            text: i18n("Redo")
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn

            onClicked: textArea.redo()
        },
        ToolButton {
            icon.name: "search"
            text: i18nc("@action:button", "Search Note")
            display: AbstractButton.IconOnly
            visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns
            checkable: true
            checked: searchBar.isSearchOpen
            onClicked: {
                if(searchBar.isSearchOpen === true) {
                    closeSearch()
                } else {
                    openSearch()
                }
            }

            ToolTip.text: i18nc("@info:tooltip", "Search in Note")
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        },
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
            text: i18n("Focus Mode")
            visible: wideScreen && pageStack.columnView.columnResizeMode === Kirigami.ColumnView.FixedColumns

            Behavior on columnWidth {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration * 2
                    easing.type: Easing.InOutQuart
                }
            }

            onClicked: {
                Config.fillWindow = !Config.fillWindow;
            }
            onColumnWidthChanged: pageStack.defaultColumnWidth = columnWidth

            Shortcut {
                sequence: "Ctrl+R"

                onActivated: Config.fillWindow = !Config.fillWindow
            }
        },                                           
        ToolButton {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: text
            ToolTip.visible: hovered
            checkable: true
            checked: true
            display: AbstractButton.IconOnly
            icon.name: "window-restore-symbolic"
            text: i18nc("@action:menu", "Exit Full Screen")
            visible: editPage.Window.window.visibility === Window.FullScreen

            onClicked: editPage.Window.window.showNormal()
        },
        Button {
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.text: i18n("Switch editor to preview mode")
            ToolTip.visible: hovered
            icon.name: "code-context-symbolic"
            checkable: true
            checked: true
            text: i18n("Source View")
            padding: 0
            flat: true
            spacing: Kirigami.Units.smallSpacing

            onClicked: {
                NavigationController.sourceMode = !NavigationController.sourceMode
            }
        }
    ]

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
        onUndo: root.textArea.undo()
        onSelectCursor: (start, end) => {
            root.textArea.select(start, end);
        }
    }
}
