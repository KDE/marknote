// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

pragma Singleton
import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.marknote // For CommandManager

Item {
    id: root

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

}
