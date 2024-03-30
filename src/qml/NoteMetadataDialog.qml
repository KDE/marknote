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

Controls.Dialog {
    id: root

    enum Mode {
        Edit,
        Add
    }

    property int mode: NoteMetadataDialog.Mode.Add
    property alias name: nameInput.text
    required property var model

    parent: applicationWindow().overlay

    x: Math.round((parent.width - width) / 2)
    y: Math.round(parent.height / 3)

    width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 15)

    title: mode === NotebookMetadataDialog.Mode.Add ? i18nc("@title:window", "New Note") : i18nc("@title:window", "Edit Note")

    background: Components.DialogRoundedBackground {}

    modal: true
    focus: true

    padding: 0

    onOpened: nameInput.forceActiveFocus()

    header: Kirigami.Heading {
        text: root.title
        elide: Controls.Label.ElideRight
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing
    }

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormTextFieldDelegate {
            id: nameInput

            label: i18nc("@label:textbox Note name", "Name:")
            leftPadding: Kirigami.Units.largeSpacing * 2
            rightPadding: Kirigami.Units.largeSpacing * 2

            onAccepted: root.accepted()
        }
    }

    footer: Controls.DialogButtonBox {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2

        standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel
    }

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
}
