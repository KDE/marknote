import QtQuick 2.1
import org.kde.kirigami 2.15 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

Kirigami.ScrollablePage {
    Kirigami.Theme.colorSet: Kirigami.Theme.Window
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
            onClicked:{
                notesModel.deleteNote(optionPopup.path)
                optionPopup.dismiss()
                }
            }
        MenuItem {
            text: "rename"
            onClicked: {
                renameSheet.open()
                renameSheet.path = optionPopup.path
                }
            }
        }

    Kirigami.OverlayDrawer {
        property string path
        id: optionDrawer
        width: appwindow.width
        edge: Qt.BottomEdge
        parent: applicationWindow().overlay

        ColumnLayout{
            id: contents
            anchors.fill: parent
            spacing: 0

            Kirigami.Icon {
                Layout.margins: Kirigami.Units.smallSpacing
                source: "arrow-down"
                implicitWidth: Kirigami.Units.gridUnit
                implicitHeight: Kirigami.Units.gridUnit
                Layout.alignment: Qt.AlignHCenter
            }
            ToolButton{
                Layout.fillWidth: true
                icon.name: "delete"
                text: "Delete"
                onClicked: {
                    notesModel.deleteNote(optionDrawer.path)
                    optionDrawer.close()
                }

            }
            ToolButton{
                Layout.fillWidth: true
                icon.name: "document-edit"
                text: "Rename"
                onClicked: {
                    renameSheet.open()
                    renameSheet.path = optionDrawer.path
                }
            }

        }
    }

    Kirigami.OverlaySheet{
        id: addSheet
        parent: applicationWindow().overlay
        header: Kirigami.Heading{
            text: "New Note"
            
        }
        contentItem: Kirigami.FormLayout{
            TextField{
                id: fileNameInput
                Kirigami.FormData.label: "Note Name:"
            }
        }
        footer: RowLayout {
            ToolButton{
                text: "Cancel"
                Layout.fillWidth: true
                onClicked:{
                    addSheet.close()
                }
            }
            Kirigami.Separator{
                Layout.fillHeight: true
                width: 1
            }
            ToolButton{
                text: "Add"
                Layout.fillWidth: true
                onClicked:{
                    addSheet.close()
                    notesModel.addNote(fileNameInput.text)
                }
            }
        }
    }

    Kirigami.OverlaySheet{
        property string path
        id: renameSheet
        header: Kirigami.Heading{
            text: "Rename Note"
        }
        contentItem: Kirigami.FormLayout{
            TextField{
                id: renameInput
                Kirigami.FormData.label: "New Name:"
            }
        }
        footer: Button{
            text: "Rename"
            onClicked:{
                renameSheet.close()
                notesModel.renameNote(renameSheet.path, renameInput.text)
            }
        }
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

            label: name
            subtitle: Qt.formatDateTime(date, Qt.SystemLocaleDate)
            onClicked: pageStack.push("qrc:/EditPage.qml", {"path": path, "name": name})
            ToolButton{
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
