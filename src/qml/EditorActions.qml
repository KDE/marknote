// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

pragma Singleton
import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.marknote // For CommandManager
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

Item {
    id: root

    KQuickControlsAddons.Clipboard {
        id: clipboard
    }

    property Kirigami.Action deleteBlockAction: Kirigami.Action {
        text: i18n("Delete")
        icon.name: "edit-delete"
        enabled: CommandManager.model !== null && CommandManager.model.hasSelection
        
        onTriggered: {
            if (CommandManager.model && CommandManager.model.hasSelection) {
                var blocks = CommandManager.model.selectedBlocks();
                CommandManager.removeBlocks(blocks);
            }
        }
    }

    property Kirigami.Action undoAction: Kirigami.Action {
        text: i18n("Undo")
        icon.name: "edit-undo"
        enabled: CommandManager.canUndo
        onTriggered: CommandManager.undo()
    }

    property Kirigami.Action redoAction: Kirigami.Action {
        text: i18n("Redo")
        icon.name: "edit-redo"
        enabled: CommandManager.canRedo
        onTriggered: CommandManager.redo()
    }

    property Kirigami.Action copyAction: Kirigami.Action {
        text: i18n("Copy")
        icon.name: "edit-copy"
        enabled: CommandManager.model !== null && CommandManager.model.hasSelection
        
        onTriggered: {
            if (CommandManager.model && CommandManager.model.hasSelection) {
                var blocks = CommandManager.model.selectedBlocks();
                var mdText = CommandManager.blocksToMarkdown(blocks);
                clipboard.content = mdText;
            }
        }
    }

    property var activePopup: null

    function handleKeyEvent(event): bool {
        if (!activePopup || !activePopup.opened) {
            return false;
        }

        if (event.key === Qt.Key_Up) {
            if (activePopup.moveSelectionUp) {
                activePopup.moveSelectionUp();
            }
            event.accepted = true;
            return true;
        } else if (event.key === Qt.Key_Down) {
            if (activePopup.moveSelectionDown) {
                activePopup.moveSelectionDown();
            }
            event.accepted = true;
            return true;
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Tab) {
            if (activePopup.selectCurrent) {
                activePopup.selectCurrent();
            }
            event.accepted = true;
            return true;
        } else if (event.key === Qt.Key_Escape) {
            activePopup.close();
            event.accepted = true;
            return true;
        }

        return false;
    }
}
