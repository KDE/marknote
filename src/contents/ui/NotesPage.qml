import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

import "components"

Kirigami.ScrollablePage {
    id: root
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}

    ActionButton {
        visible: Kirigami.Settings.isMobile
        parent: root.overlay
        x: root.width - width - margin
        y: root.height - height - pageStack.globalToolBar.preferredHeight - margin
        singleAction: Kirigami.Action {
            text: i18n("add Note")
            icon.name: "list-add"
            onTriggered: addSheet.open()
        }
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        ToolButton {
            visible: !Kirigami.Settings.isMobile
            id: addButton
            icon.name: "list-add"
            text: "Add"
            onClicked: addSheet.open()
            display: AbstractButton.IconOnly

        }
        Kirigami.Heading {
            id: heading
            text: i18n("Your Notes")
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            horizontalAlignment: Kirigami.Settings.isMobile? Text.AlignLeft: Text.AlignHCenter
        }
        Kirigami.SearchField {
            id: search
            visible: false
            Layout.fillWidth: true
            onTextChanged: filterModel.setFilterFixedString(search.text )
        }
        ToolButton {
            id: searchButton
            icon.name: "search"
            text: "Search"
            display: AbstractButton.IconOnly

            onClicked:{
                if (!search.visible){
                    search.visible = true
                    heading.visible = false
                    addButton.visible = false
                    searchButton.icon.name = "draw-arrow-back"
                } else {
                    search.visible = false
                    heading.visible = true
                    if (!Kirigami.Settings.isMobile) {addButton.visible = true}
                    search.clear()
                    searchButton.icon.name = "search"
                }

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

        model: SortFilterModel {
            id: filterModel
            filterCaseSensitivity: Qt.CaseInsensitive
            filterRole: NotesModel.Name
            sourceModel: NotesModel {
                id: notesModel
            }
        }

        delegate: Kirigami.AbstractListItem {
            id: delegateItem
            required property string name;
            required property string path;
            required property date date;
            separatorVisible: false



            contentItem: RowLayout{
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    RowLayout {
                        Layout.fillWidth: true
                        id: renameLayout
                        visible: false
                        TextField {
                            Layout.fillWidth: true
                            id: renameField
                            text: name
                            onAccepted: notesModel.renameNote(path, text)
                        }
                        Button {
                            icon.name: "answer-correct"
                            onClicked: notesModel.renameNote(path, renameField.text)
                        }
                    }
                    Label {
                        id:nameLabel
                        Layout.topMargin: 7
                        Layout.bottomMargin: 7
                        text: name
                        Layout.fillWidth: true
                        elide: Qt.ElideRight

                    }
                    Label {
                        text: Qt.formatDateTime(date, Qt.SystemLocaleDate)
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        elide: Qt.ElideRight

                    }
                }
                ToolButton{
                    icon.name: "overflow-menu"


                    onClicked: {
                        if(Kirigami.Settings.isMobile){
                            optionDrawer.open()

                        }else{

                            optionPopup.popup()

                    }
                }

                BottomDrawer {
                        id: optionDrawer


                        drawerContentItem: ColumnLayout{
                            id: contents
                            spacing: 0
                            Kirigami.BasicListItem {
                                label: i18n("Delete")
                                icon: "delete"
                                onClicked: {
                                    notesModel.deleteNote(delegateItem.path)
                                    optionDrawer.close()

                                }
                            }
                            Kirigami.BasicListItem {
                                label: i18n("Rename")
                                icon: "document-edit"
                                onClicked: {
                                    if (!renameLayout.visible) {
                                        renameLayout.visible = true
                                        nameLabel.visible = false
                                        optionDrawer.close()

                                    } else {
                                        renameLayout.visible = false
                                        nameLabel.visible = true
                                        optionDrawer.close()

                                    }
                                }
                            }
                            Item{
                                Layout.fillHeight: true
                            }
                        }
                    }


                Menu {
                    id: optionPopup
                    MenuItem {
                        text: "delete"
                        icon.name: "delete"
                        onClicked:{
                            notesModel.deleteNote(delegateItem.path)
                            optionPopup.dismiss()
                            }
                        }
                    MenuItem {
                        text: "rename"
                        icon.name: "edit-rename"
                        onClicked: {
                            if (!renameLayout.visible) {
                                renameLayout.visible = true
                                nameLabel.visible = false
                            } else {
                                renameLayout.visible = false
                                nameLabel.visible = true
                            }
                        }
                    }
                }
            }
        }




        onClicked: pageStack.push("qrc:/EditPage.qml", {"path": path, "name": name, "objectName": name})
        highlighted: pageStack.currentItem && pageStack.currentItem.objectName === name
        topPadding: Kirigami.Units.mediumSpacing
        bottomPadding: Kirigami.Units.mediumSpacing



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
