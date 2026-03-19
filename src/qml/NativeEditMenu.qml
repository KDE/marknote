// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform as Labs
import QtQuick
import QtQuick.Window
import QtQuick.Controls

import org.kde.ki18n

Labs.Menu {
    id: editMenu
    title: KI18n.i18nc("@action:menu", "Edit")

    property var _window: ApplicationWindow.window

    property Connections _textInputConnection: Connections {
        target: editMenu._window // <-- Fixed here
        function onActiveFocusItemChanged() {
            // <-- Fixed 2 spots in the line below
            if (editMenu._window.activeFocusItem instanceof TextEdit || editMenu._window.activeFocusItem instanceof TextInput) {
                // <-- Fixed 1 spot in the line below
                editMenu.field = editMenu._window.activeFocusItem;
            }
        }
    }

    property var field: null

    Component.onCompleted: {
        for (let i in additionalMenuItems) {
            editMenu.addItem(additionalMenuItems[i])
        }
        for (let j in _menuItems) {
            editMenu.addItem(_menuItems[j])
        }
    }

    default property list<QtObject> additionalMenuItems

    property list<QtObject> _menuItems: [
        Labs.MenuItem {
            enabled: editMenu.field !== null && editMenu.field.canUndo
            text: KI18n.i18nc("text editing menu action", "Undo")
            shortcut: StandardKey.Undo
            onTriggered: {
                editMenu.field.undo()
                editMenu.close()
            }
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null && editMenu.field.canRedo
            text: KI18n.i18nc("text editing menu action", "Redo")
            shortcut: StandardKey.Redo
            onTriggered: {
                editMenu.field.undo()
                editMenu.close()
            }
        },

        Labs.MenuSeparator {
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null && editMenu.field.selectedText
            text: KI18n.i18nc("text editing menu action", "Cut")
            shortcut: StandardKey.Cut
            onTriggered: {
                editMenu.field.cut()
                editMenu.close()
            }
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null && editMenu.field.selectedText
            text: KI18n.i18nc("text editing menu action", "Copy")
            shortcut: StandardKey.Copy
            onTriggered: {
                editMenu.field.copy()
                editMenu.close()
            }
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null && canPaste
            text: KI18n.i18nc("text editing menu action", "Paste")
            shortcut: StandardKey.Paste

            property bool canPaste: {
                if (editMenu._window?.currentDocument !== undefined) {
                    return editMenu._window.currentDocument.canPaste;
                }
                if (editMenu.field !== null) {
                    return editMenu.field.canPaste;
                }
                return false;
            }

            onTriggered: {
                if (typeof editMenu._window?.currentDocument?.pasteFromClipboard === 'function') {
                    editMenu._window.currentDocument.pasteFromClipboard()
                } else if (editMenu.field !== null) {
                    editMenu.field.paste()
                }
                editMenu.close()
            }
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null && editMenu.field.selectedText !== ""
            text: KI18n.i18nc("text editing menu action", "Delete")
            shortcut: ""
            onTriggered: {
                editMenu.field.remove(editMenu.field.selectionStart, editMenu.field.selectionEnd)
                editMenu.close()
            }
        },

        Labs.MenuSeparator {
        },

        Labs.MenuItem {
            enabled: editMenu.field !== null
            text: KI18n.i18nc("text editing menu action", "Select All")
            shortcut: StandardKey.SelectAll
            onTriggered: {
                editMenu.field.selectAll()
                editMenu.close()
            }
        }
    ]
}
