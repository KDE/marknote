// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Qt.labs.platform
import QtQuick.Dialogs

import org.kde.marknote
import org.kde.iconthemes as KIconThemes
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami

FormCard.FormCardDialog {
    id: root

    enum Mode {
        Maildir,
        KNotes
    }

    property int mode: ImportMaildirDialog.Mode.Maildir
    property alias name: nameInput.text
    property alias iconName: iconButton.iconName
    property alias color: colorButton.color
    property NoteBooksModel model

    title: mode === ImportMaildirDialog.Mode.Maildir ? i18nc("@title:window", "Import from Maildir") : i18nc("@title:window", "Import from KNotes")

    FormCard.FormTextFieldDelegate {
        id: nameInput

        label: i18nc("@label:textbox Notebook name", "Name:")
        onAccepted: root.accepted()
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.FormButtonDelegate {
        id: directoryInput

        property url path: StandardPaths.writableLocation(StandardPaths.GenericDataLocation) + '/notes'

        text: i18nc("@label:textbox Notebook name", "Maildir location:")
        description: path.toString()
        onClicked: maildirSelectDialog.open()
        visible: mode === ImportMaildirDialog.Mode.Maildir

        FolderDialog {
            id: maildirSelectDialog
            title: i18nc("@title:window", "Select a Maildir location")
            currentFolder: directoryInput.path
            onAccepted: directoryInput.path = selectedFolder
        }
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
        visible: mode === ImportMaildirDialog.Mode.Maildir
    }

    FormCard.FormIconDelegate {
        id: iconButton
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.FormColorDelegate {
        id: colorButton
    }

    MaildirImport {
        id: maildirImport
    }

    onOpened: nameInput.forceActiveFocus()

    onClosed: {
        color = "";
        name = "";
        iconName = "addressbook-details"
    }

    onAccepted: {
        const destination = root.model.addNoteBook(root.name, root.iconName, root.color);
        maildirImport.import(directoryInput.path, 'file:' + destination);
        NavigationController.notebookPath = destination;

        close();
    }

    footer: Controls.DialogButtonBox {
        standardButtons: Controls.Dialog.Cancel
        Controls.Button {
            text: i18nc("@action:button", "Import")
            enabled: nameInput.text.length > 0
            Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
        }
    }
}
