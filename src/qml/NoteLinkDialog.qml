// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardDialog {
    id: root

    property alias noteName: noteNameField.text
    property alias noteAlias: noteAliasField.text

    // Internal state to track if the user has intentionally overridden the alias
    property bool _aliasManuallyEdited: false

    onOpened: {
        // If an existing link is loaded and the alias differs from the name,
        // treat it as already manually edited so we don't overwrite it.
        _aliasManuallyEdited = (noteAliasField.text !== noteNameField.text && noteAliasField.text.length > 0);

        // UX: Auto-focus the first field so the user can immediately start typing
        noteNameField.forceActiveFocus();
    }

    onClosed: {
        noteNameField.text = "";
        noteAliasField.text = "";
        _aliasManuallyEdited = false;
    }

    Component.onCompleted: {
        // UX: Disable the 'OK' button if the note name is empty
        const okBtn = standardButton(QQC2.Dialog.Ok);
        if (okBtn) {
            okBtn.enabled = Qt.binding(() => noteNameField.text.trim().length > 0);
        }
    }

    title: i18nc("@title:window", "Insert Note Link")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    FormCard.FormTextFieldDelegate {
        id: noteNameField

        label: i18nc("@label:textbox", "Note Name:")

        Accessible.description: i18nc("@info:whatsthis", "The name of the note you want to link to.")

        onTextChanged: {
            // Only sync if the user hasn't explicitly set a custom alias
            if (!root._aliasManuallyEdited) {
                noteAliasField.text = text;
            }
        }

        onAccepted: root.accept()
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: noteAliasField

        label: i18nc("@label:textbox", "Display Text:")

        Accessible.description: i18nc("@info:whatsthis", "Optional alternate text to display instead of the note name.")

        onTextChanged: {
            // activeFocus ensures this only triggers when the user is actively typing,
            // not when the text is updated programmatically via noteNameField.
            if (activeFocus) {
                root._aliasManuallyEdited = true;
            }
        }

        onAccepted: root.accept()
    }
}
