import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

import org.kde.marknote 1.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons
import QtQuick.Dialogs 1.0 as QtDialogs

Kirigami.Dialog{
    id: root
    title: "New Notebook"
    property NoteBooksModel model
    property string notebookColor
    padding: Kirigami.Units.largeSpacing
    onOpened: nameInput.forceActiveFocus()
    contentItem: ColumnLayout {
        spacing: 20
        KQuickAddons.IconDialog {
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
                placeholderText: i18n("Notebook Name")
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
            iconName: "list-add"

            onTriggered: {
                root.model.addNoteBook(nameInput.text, buttonIcon.source !== "" ? buttonIcon.source : "addressbook-details" , root.notebookColor)
                close()
                if (model.rowCount() === 1) {
                    pageStack.clear()
                    pageStack.replace(["qrc:/NotesPage.qml","qrc:/EditPage.qml"], {
                        path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                        notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
                    })
                }
                notebookColor = ""
                nameInput.clear()
                buttonIcon.source = "addressbook-details"
            }
        }
    ]
}
