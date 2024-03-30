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

    property int index: -1
    property int mode: NotebookMetadataDialog.Mode.Add
    property alias name: nameInput.text
    property alias iconName: buttonIcon.source
    property alias color: colorRect.color

    property NoteBooksModel model

    title: mode === NotebookMetadataDialog.Mode.Add ? i18nc("@title:window", "New Notebook") : i18nc("@title:window", "Edit Notebook")

    standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

    FormCard.FormTextFieldDelegate {
        id: nameInput

        label: i18nc("@label:textbox Notebook name", "Name:")
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.AbstractFormDelegate {
        id: iconButton

        text: i18nc("@action:button", "Icon")
        icon.name: "color-picker"
        onClicked: iconDialog.open()

        contentItem: RowLayout {
            spacing: 0

            Kirigami.Icon {
                source: "preferences-desktop-emoticons"
                Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                Layout.fillWidth: true
                text: iconButton.text
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                Accessible.ignored: true // base class sets this text on root already
            }

            Kirigami.Icon {
                id: buttonIcon

                source: "addressbook-details-symbolic"
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            }

            FormCard.FormArrow {
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                direction: Qt.RightArrow
                visible: root.background.visible
            }
        }

        KIconThemes.IconDialog {
            id: iconDialog
            onIconNameChanged: buttonIcon.source = iconName
        }
    }

    FormCard.FormDelegateSeparator {
        Layout.fillWidth: true
    }

    FormCard.AbstractFormDelegate {
        id: colorButton

        text: i18nc("@action:button", "Color")
        icon.name: "color-picker"
        onClicked: colorDialog.open()

        contentItem: RowLayout {
            spacing: 0

            Kirigami.Icon {
                source: "color-picker"
                Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                Layout.fillWidth: true
                text: colorButton.text
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                Accessible.ignored: true // base class sets this text on root already
            }

            Rectangle {
                id: colorRect
                radius: height
                color: "transparent"
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            }

            FormCard.FormArrow {
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                direction: Qt.RightArrow
                visible: root.background.visible
            }
        }


        ColorDialog {
            id: colorDialog
            onAccepted: {
                root.colorRect.color = colorDialog.color;
            }
        }
    }

    onOpened: nameInput.forceActiveFocus()

    onClosed: {
        color = "";
        name = "";
        iconName = "addressbook-details"
    }

    onAccepted: {
        if (mode == NotebookMetadataDialog.Mode.Add) {
            root.model.addNoteBook(root.name, root.iconName, root.color);
        } else {
            root.model.editNoteBook(root.index, root.name, root.iconName, root.color);
        }

        close();

        if (model.rowCount() === 1) {
            pageStack.clear()
            pageStack.replace([
                Qt.createComponent("org.kde.marknote", "NotesPage"),
                Qt.createComponent("org.kde.marknote", "EditPage")
            ], {
                path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
            });
        }
    }

}
