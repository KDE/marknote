// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

import "components"

Kirigami.ScrollablePage {
    id: root
    property bool wideScreen: applicationWindow().width >= 600

    property string path
    property string notebookName
    Component.onCompleted: if (!Kirigami.Settings.isMobile) {
        notesModel.rowCount() !== 0 ? pageStack.push("qrc:/EditPage.qml",{
        path: notesModel.data(notesModel.index(0, 0), NotesModel.Path),
        name: notesModel.data(notesModel.index(0, 0), NotesModel.Name),
        objectNameW: notesModel.data(notesModel.index(0, 0), NotesModel.Name)})
        :
        pageStack.push("qrc:/EditPage.qml",{name: ""})
        }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}

    ActionButton {
        visible: Kirigami.Settings.isMobile
        parent: root.overlay
        x: root.width - width - Kirigami.Units.gridUnit
        y: root.height - height - pageStack.globalToolBar.preferredHeight - Kirigami.Units.gridUnit
        text: i18n("Add note")
        icon.name: "list-add"
        onClicked: addSheet.open()
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        ToolButton {
            id: addButton
            visible: !Kirigami.Settings.isMobile
            icon.name: "list-add"
            text: i18n("New Note (%1)", addShortcut.nativeText)
            onClicked: addSheet.open()
            display: AbstractButton.IconOnly

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: hovered
            ToolTip.text: text

            Shortcut {
                id: addShortcut
                sequence: "Ctrl+N"
                onActivated: addButton.clicked()
            }
        }
        Kirigami.Heading {
            id: heading

            visible: wideScreen
            text: root.notebookName
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            horizontalAlignment: Text.AlignHCenter
        }
        ToolButton {
            id: headingButton

            visible: !wideScreen
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: applicationWindow().openBottomDrawer()
            contentItem: RowLayout{
                Item {
                    visible: !Kirigami.Settings.isMobile
                    Layout.fillWidth: true
                }

                Kirigami.Heading {
                    type: Kirigami.Heading.Type.Primary
                    text: root.notebookName
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                }

                Kirigami.Icon {
                    source: "go-down-symbolic"
                    implicitHeight: Kirigami.Units.gridUnit
                }

                Item { Layout.fillWidth: true }

            }
        }
        Kirigami.SearchField {
            id: search
            visible: false
            Layout.fillWidth: true
            Shortcut {
                id: cancelShortcut
                sequence: StandardKey.Cancel
                onActivated: if (search.visible) {searchButton.clicked()}
            }
            onTextChanged: filterModel.setFilterFixedString(search.text )
        }
        ToolButton {
            id: searchButton
            icon.name: "search"
            text: search.visible ? i18n("Exit Search (%1)", cancelShortcut.nativeText) : i18n("Search notes (%1)", searchShortcut.nativeText)
            display: AbstractButton.IconOnly

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: hovered
            ToolTip.text: text

            onClicked:{
                if (!search.visible){
                    search.visible = true
                    wideScreen? heading.visible = false : headingButton.visible = false
                    addButton.visible = false
                    searchButton.icon.name = "draw-arrow-back"
                    search.forceActiveFocus()
                } else {
                    search.visible = false
                    wideScreen? heading.visible = true : headingButton.visible = true
                    if (!Kirigami.Settings.isMobile) {addButton.visible = true}
                    search.clear()
                    searchButton.icon.name = "search"
                }
            }

            Shortcut {
                id: searchShortcut
                sequence: StandardKey.Find
                onActivated: if (!search.visible) {
                    searchButton.clicked()
                }
            }
        }
    }
    Kirigami.Dialog{
        id: addSheet
        title: i18n("New Note")
        padding: Kirigami.Units.largeSpacing
        contentItem: Kirigami.FormLayout{
            TextField{
                id: fileNameInput
                Kirigami.FormData.label: i18n("Note name:")
                onAccepted: addAction.triggered()
            }
        }
        standardButtons: Kirigami.Dialog.Cancel
        onOpened: fileNameInput.forceActiveFocus()
        customFooterActions: [
            Kirigami.Action {
                id: addAction
                text: i18n("Add")
                iconName: "list-add"
                onTriggered: {
                    addSheet.close()
                    notesModel.addNote(fileNameInput.text)
                }
            }
        ]
    }
    Kirigami.Dialog {
        id: removeDialog

        property string notePath
        property string noteName

        standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.Cancel
        title: i18n("Delete Note")

        onRejected: close()
        onAccepted: notesModel.deleteNote(notePath)

        RowLayout {
            Kirigami.Icon {
                source: "dialog-warning"
                Layout.margins: Kirigami.Units.largeSpacing
            }

            Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
                text: i18n("Are you sure you want to delete the note <b> %1 </b>?", removeDialog.noteName)
                wrapMode: Text.WordWrap
            }
        }
    }

    ListView {
        id: notesList

        model: SortFilterModel {
            id: filterModel
            filterCaseSensitivity: Qt.CaseInsensitive
            filterRole: NotesModel.Name
            sourceModel: NotesModel {
                id: notesModel
                path: root.path
            }
        }

        delegate: Kirigami.AbstractListItem {
            id: delegateItem

            required property string name;
            required property string path;
            required property date date;
            required property int index;

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

                    onClicked: if (Kirigami.Settings.isMobile) {
                        optionDrawer.open()
                    } else {
                        optionPopup.popup()
                    }

                    BottomDrawer {
                        id: optionDrawer

                        drawerContentItem: ColumnLayout{
                            id: contents
                            spacing: 0

                            Kirigami.BasicListItem {
                                label: i18n("Rename Note")
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
                            Kirigami.BasicListItem {
                                label: i18n("Delete Note")
                                icon: "delete"
                                onClicked: {
                                    removeDialog.noteName = delegateItem.name
                                    removeDialog.notePath = delegateItem.path
                                    removeDialog.open()
                                    optionDrawer.close()
                                }
                            }
                            Item { height: Kirigami.Units.largeSpacing * 3}
                        }
                    }

                    Menu {
                        id: optionPopup

                        MenuItem {
                            text: i18n("Rename Note")
                            icon.name: "edit-rename"
                            onClicked: if (!renameLayout.visible) {
                                renameLayout.visible = true
                                nameLabel.visible = false
                            } else {
                                renameLayout.visible = false
                                nameLabel.visible = true
                            }
                        }
                        MenuItem {
                            text: i18n("Delete Note")
                            icon.name: "delete"
                            onClicked:{
                                removeDialog.noteName = delegateItem.name
                                removeDialog.notePath = delegateItem.path
                                removeDialog.open()
                                optionPopup.dismiss()
                            }
                        }
                    }
                }
            }

            onClicked: pageStack.push("qrc:/EditPage.qml", {
                path: path,
                name: name,
                objectName: name
            })
            highlighted: pageStack.currentItem && pageStack.currentItem.objectName === name
            topPadding: Kirigami.Units.mediumSpacing
            bottomPadding: Kirigami.Units.mediumSpacing
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            icon.name: "note"
            visible: notesList.count === 0
            text: i18n("Add a note!")
            helpfulAction: Kirigami.Action {
                icon.name: "list-add"
                text: i18n("Add")
                onTriggered: addSheet.open()
            }
        }
    }
}
