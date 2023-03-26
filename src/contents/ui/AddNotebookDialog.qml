import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

import org.kde.marknote 1.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

Kirigami.Dialog{
    id: root
    title: "New Notebook"
    property NoteBooksModel model

    padding: Kirigami.Units.largeSpacing
    contentItem: ColumnLayout {
        spacing: 20
        KQuickAddons.IconDialog {
            id: iconDialog
            onIconNameChanged: buttonIcon.source = iconName
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
                placeholderText: "Notebook Name"
            }
            Button { icon.name: "color-management"}
        }

    }
    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Add")
            iconName: "list-add"
            onTriggered: {
                root.model.addNoteBook(nameInput.text)
                close()
                if (model.rowCount() === 1) {pageStack.replace(
                        ["qrc:/NotesPage.qml","qrc:/EditPage.qml"],
                    {
                    path: noteBooksModel.data(noteBooksModel.index(0, 0), NotesModel.Path),
                    notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NotesModel.Name)

                    }
                    )
                }

            }
        }
    ]
}
