// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.marknote
import org.kde.iconthemes as KIconThemes
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami

FormCard.FormCardDialog {
    id: root

    enum Mode {
        Edit,
        Add
    }

    property int mode: NoteMetadataDialog.Mode.Add
    property alias name: nameInput.text
    required property var model

    title: mode === NotebookMetadataDialog.Mode.Add ? i18nc("@title:window", "New Note") : i18nc("@title:window", "Edit Note")
    standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

    onOpened: nameInput.forceActiveFocus()

    onClosed: {
        name = "";
    }

    onAccepted: {
        if (mode == NoteMetadataDialog.Mode.Add) {
            let path = root.model.addNote(root.name);

            pageStack.pop();
            pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"), {
                name: root.name,
                path: path,
            });
        }

        close();
    }

    FormCard.FormTextFieldDelegate {
        id: nameInput

        label: i18nc("@label:textbox Note name", "Name:")

        onAccepted: root.accepted()
    }
}
