// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>

import QtQuick
import QtQuick.Controls as Controls
import org.kde.marknote

Controls.Menu {
    id: root

    property string path
    property string name
    property NoteBooksModel model

    Controls.MenuItem {
        text: i18nc("@action:inmenu", "Edit Notebook")
        icon.name: "edit-entry-symbolic"
        onTriggered: {
            const editComponent = Qt.createComponent("org.kde.marknote", "NotebookMetadataDialog");
            const editDialog = editComponent.createObject(root, {
                mode: NotebookMetadataDialog.Mode.Edit,
                path: root.path,
                name: root.name,
                color: noteBooksModel.colorForPath(root.path),
                iconName: noteBooksModel.iconNameForPath(root.path),
                model: root.model,
            });
            editDialog.open();
        }
    }

    Controls.MenuItem {
        text: i18nc("@action:inmenu", "Delete Notebook")
        icon.name: "delete"
        onTriggered: {
            const deleteComponent = Qt.createComponent("org.kde.marknote", "NotebookDeleteDialog");
            const deleteDialog = deleteComponent.createObject(root, {
                path: root.path,
                name: root.name,
                model: root.model,
            });
            deleteDialog.open();
        }
    }
}
