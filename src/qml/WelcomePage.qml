// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.marknote

Kirigami.Page {
    id: root
    property NoteBooksModel model
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}

    NotebookMetadataDialog {
        id: notebookMetadataDialog

        model: noteBooksModel
    }
    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        icon.name: "addressbook-details"
        text: i18n("Start by creating your first notebook!")
        helpfulAction: Kirigami.Action {
            icon.name: "list-add"
            text: i18n("Add Notebook")
            onTriggered: {
                notebookMetadataDialog.mode = NotebookMetadataDialog.Mode.Add;
                notebookMetadataDialog.open();
            }
        }
    }
}
