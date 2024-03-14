// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.marknote
import org.kde.iconthemes as KIconThemes
import QtQuick.Dialogs as QtDialogs

Kirigami.Dialog{
    id: root
    title: i18n("New Notebook")
    property NoteBooksModel model
    property string notebookColor
    padding: Kirigami.Units.largeSpacing
    onOpened: nameInput.forceActiveFocus()
    contentItem: ColumnLayout {
        spacing: 20
        KIconThemes.IconDialog {
            id: iconDialog
            onIconNameChanged: buttonIcon.source = iconName
        }
        QtDialogs.ColorDialog {
            id: colorDialog
            onAccepted: {
                root.notebookColor=colorDialog.color
                colorButton.palette.button = colorDialog.color
            }
        }
        Button {

            implicitHeight: Kirigami.Units.gridUnit *4
            implicitWidth: Kirigami.Units.gridUnit *4
            id: iconButton
            Layout.alignment: Qt.AlignHCenter
            onClicked: iconDialog.open()
            contentItem: Item{
                Kirigami.Icon{
                    id: buttonIcon
                    source:"addressbook-details"
                    anchors.centerIn: parent
                    height: Kirigami.Units.gridUnit*2
                    width:height
                }
            }
        }
        RowLayout {
            TextField{
                id: nameInput
                placeholderText: i18n("Notebook name")
            }
            Button {
                id: colorButton
                icon.name: "color-picker"
                onClicked: colorDialog.open()
            }
        }

    }
    standardButtons: Kirigami.Dialog.Cancel
    onRejected: {
        notebookColor = ""
        nameInput.clear()
        buttonIcon.source = "addressbook-details"

    }
    customFooterActions: [
        Kirigami.Action {
            text: i18n("Add")
            icon.name: "list-add"

            onTriggered: {
                root.model.addNoteBook(nameInput.text, buttonIcon.source !== "" ? buttonIcon.source : "addressbook-details" , root.notebookColor)
                close()
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
                notebookColor = ""
                nameInput.clear()
                buttonIcon.source = "addressbook-details"
            }
        }
    ]
}