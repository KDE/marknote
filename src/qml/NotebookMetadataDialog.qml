// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Qt.labs.platform

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

    property string path: ''
    property int mode: NotebookMetadataDialog.Mode.Add
    property alias name: nameInput.text
    property alias iconName: iconButton.iconName
    property alias color: colorButton.color

    property NoteBooksModel model

    title: mode === NotebookMetadataDialog.Mode.Add ? i18nc("@title:window", "New Notebook") : i18nc("@title:window", "Edit Notebook")

    standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

    FormCard.FormTextFieldDelegate {
        id: nameInput

        label: i18nc("@label:textbox Notebook name", "Name:")
        validator: RegularExpressionValidator {
            regularExpression: /^[^./\\][^/\\]*$/
        }
        onAccepted: root.accepted()
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.FormIconDelegate {
        id: iconButton
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.FormColorDelegate {
        id: colorButton
        color: root.mode === NotebookMetadataDialog.Mode.Edit
        ? noteBooksModel.colorForPath(root.path)
        : Kirigami.Theme.highlightColor
    }

    onOpened: nameInput.forceActiveFocus();

    onClosed: root.destroy();

    onAccepted: {
        if (nameInput.text.length === 0) {
            return;
        }

        if (mode == NotebookMetadataDialog.Mode.Add) {
            NavigationController.notebookPath = root.model.addNoteBook(root.name, root.iconName, root.color);
        } else {
            root.model.editNoteBook(root.path, root.name, root.iconName, root.color);
        }

        close();
    }

    Component.onCompleted: {
        let saveButton = root.standardButton(Controls.Dialog.Save);
        if (saveButton) {
            // This creates a persistent, declarative link between the button's state and the text length
            saveButton.enabled = Qt.binding(() => nameInput.text.length > 0);
        }
    }

    onDiscarded: root.close();
    onRejected: root.close();
}
