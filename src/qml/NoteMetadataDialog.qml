// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls

import org.kde.marknote
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n

FormCard.FormCardDialog {
    id: root

    enum Mode {
        Edit,
        Add
    }

    property int mode: NoteMetadataDialog.Mode.Add
    property alias name: nameInput.text
    required property var model

    readonly property bool duplicateName: mode === NoteMetadataDialog.Mode.Add
        && nameInput.text.length > 0
        && !!model
        && model.noteExists(nameInput.text)

    title: mode === NotebookMetadataDialog.Mode.Add ? KI18n.i18nc("@title:window", "Add New") : KI18n.i18nc("@title:window", "Edit Note")
    standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

    function updateSaveButton(): void {
        const saveButton = root.standardButton(Controls.Dialog.Save);
        if (saveButton) {
            saveButton.enabled = nameInput.text.length > 0 && !root.duplicateName;
        }
    }

    onOpened: {
        nameInput.forceActiveFocus()
        updateSaveButton()
    }

    onDuplicateNameChanged: updateSaveButton()

    onRejected: {
        root.close();
    }

    onClosed: {
        name = "";
    }

    onAccepted: {
        if (nameInput.text.length === 0 || root.duplicateName) {
            return;
        }
        if (mode === NoteMetadataDialog.Mode.Add) {
            if (typeSelector.selectedIndex === 0) {
                const createdName = root.model.addNote(root.name);
                if (createdName.length === 0) {
                    return;
                }
                NavigationController.notePath = createdName + '.md';
            } else if (typeSelector.selectedIndex === 1) {
                const createdName = root.model.addFolder(root.name);
            }
        }

        close();
    }
    FormCard.FormRadioSelectorDelegate {
        id: typeSelector
        consistentWidth: true
          actions: [
              Kirigami.Action {
                  text: i18nc("@option:radio", "Note")
                  icon.name: "note-symbolic"
              },
              Kirigami.Action {
                  text: i18nc("@option:radio", "Folder")
                  icon.name: "folder-symbolic"
              }
          ]
    }
    FormCard.FormTextFieldDelegate {
        id: nameInput

        label: KI18n.i18nc("@label:textbox Note name", "Name:")
        validator: RegularExpressionValidator {
            regularExpression: /^[^./\\][^/\\]*$/
        }
        status: root.duplicateName ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
        statusMessage: root.duplicateName ? KI18n.i18nc("@info", "A note or folder with this name already exists.") : ""
        onTextChanged: root.updateSaveButton()

        onAccepted: root.accepted()
    }
}
