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
            const component = Qt.createComponent("org.kde.marknote", "NotebookMetadataDialog");
            const dialog = component.createObject(root, {
                mode: NotebookMetadataDialog.Mode.Edit,
                path: root.path,
                name: root.name,
                color: noteBooksModel.colorForPath(root.path),
                iconName: noteBooksModel.iconNameForPath(root.path),
                model: root.model,
            });
            dialog.open();
        }
    }

    Controls.MenuItem {
        action: NotebookDeleteAction {}
    }
}
