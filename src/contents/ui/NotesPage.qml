import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

import "components"

Kirigami.ScrollablePage {
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}
    title: i18n("Your Notes")

    actions.main: Kirigami.Action {
        icon.name: "list-add"
        text: "Add"
        onTriggered: addSheet.open()
    }
    Menu {
        property string path
        id: optionPopup
        MenuItem {
            text: "delete"
            icon.name: "delete"
            onClicked:{
                notesModel.deleteNote(optionPopup.path)
                optionPopup.dismiss()
                }
            }
        MenuItem {
            text: "rename"
            icon.name: "edit-rename"
            onClicked: {
                renameSheet.open()
                renameSheet.path = optionPopup.path
                }
            }
        }
    BottomDrawer {
        id: optionDrawer
        property string path

        drawerContentItem: ColumnLayout{
            id: contents
            spacing: 0
            Kirigami.BasicListItem{
                label: i18n("Delete")
                icon: "delete"
                onClicked: {
                    notesModel.deleteNote(optionDrawer.path)
                    optionDrawer.close()

                }
            }
            Kirigami.BasicListItem{
                label: i18n("Rename")
                icon: "document-edit"
                onClicked: {
                    renameSheet.open()
                    renameSheet.path = optionDrawer.path
                    optionDrawer.close()

                }
            }
            Item{
                Layout.fillHeight: true
            }
        }
    }


    Kirigami.Dialog{
        id: addSheet
        title: "New Note"
        padding: Kirigami.Units.largeSpacing
        contentItem: Kirigami.FormLayout{
            TextField{
                id: fileNameInput
                Kirigami.FormData.label: "Note Name:"
            }
        }
        standardButtons: Kirigami.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Add")
                iconName: "list-add"
                onTriggered: {
                    addSheet.close()
                    notesModel.addNote(fileNameInput.text)
                }
            }
        ]
    }
    Kirigami.Dialog{
        id: renameSheet
        property string path
        title: "Rename Note"
        padding: Kirigami.Units.largeSpacing
        contentItem: Kirigami.FormLayout{
            TextField{
                id: renameInput
                Kirigami.FormData.label: "New Name:"
            }
        }
        standardButtons: Kirigami.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Rename")
                iconName: "edit-rename"
                onTriggered: {
                    renameSheet.close()
                    notesModel.renameNote(renameSheet.path, renameInput.text)
                }
            }
        ]
    }


    ListView {
        id: notesList

        model: NotesModel{
            id: notesModel
        }

        delegate: Kirigami.BasicListItem {
            required property string name;
            required property string path;
            required property date date;
            separatorVisible: false
            label: name
            subtitle: Qt.formatDateTime(date, Qt.SystemLocaleDate)
            onClicked: pageStack.push("qrc:/EditPage.qml", {"path": path, "name": name})
            textSpacing: Kirigami.Units.mediumSpacing
            topPadding: Kirigami.Units.mediumSpacing
            bottomPadding: Kirigami.Units.mediumSpacing
            trailing: ToolButton{
                icon.name: "overflow-menu"


                onClicked: {
                    if(Kirigami.Settings.isMobile){
                        optionDrawer.path = path
                        optionDrawer.open()

                    }else{

                        optionPopup.path = path
                        optionPopup.popup()

                }
            }
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)

        visible: notesList.count === 0
        text: "Add something to me!"
        helpfulAction: Kirigami.Action {
            icon.name: "list-add"
            text: "Add"
            onTriggered: addSheet.open()
            }
        }
    }
}
