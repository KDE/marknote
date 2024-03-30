// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kitemmodels
import org.kde.marknote
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components

import "components"

Kirigami.ScrollablePage {
    id: root
    property bool wideScreen: applicationWindow().width >= 600

    property string path
    property string notebookName
    Component.onCompleted: if (!Kirigami.Settings.isMobile) {
        if (notesModel.rowCount() !== 0) {
            pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"), {
                path: notesModel.data(notesModel.index(0, 0), NotesModel.Path),
                name: notesModel.data(notesModel.index(0, 0), NotesModel.Name),
                objectName: notesModel.data(notesModel.index(0, 0), NotesModel.Name)
            });
        } else {
            pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"), {
                name: "",
            });
        }
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
        onClicked: noteMetadataDialog.open()
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        ToolButton {
            id: addButton
            visible: !Kirigami.Settings.isMobile
            icon.name: "list-add"
            text: i18n("New Note (%1)", addShortcut.nativeText)
            onClicked: noteMetadataDialog.open()
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

    NoteMetadataDialog {
        id: noteMetadataDialog

        model: notesModel
    }

    Components.MessageDialog {
        id: removeDialog

        property string notePath
        property string noteName

        dialogType: Components.MessageDialog.Warning
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 20)
        height: implicitHeight
        title: i18nc("@title:window", "Delete Note")
        onRejected: close()
        onAccepted: notesModel.deleteNote(notePath)
        standardButtons: Dialog.Yes | Dialog.Cancel

        contentItem: Label {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            text: i18n("Are you sure you want to delete the note <b> %1 </b>?", removeDialog.noteName)
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            leftPadding: Kirigami.Units.largeSpacing * 2
            rightPadding: Kirigami.Units.largeSpacing * 2
            bottomPadding: Kirigami.Units.largeSpacing * 2
            topPadding: Kirigami.Units.largeSpacing * 2

            standardButtons: removeDialog.standardButtons
        }
    }

    ListView {
        id: notesList

        currentIndex: -1

        model: KSortFilterProxyModel {
            id: filterModel
            filterCaseSensitivity: Qt.CaseInsensitive
            filterRole: NotesModel.Name
            sourceModel: NotesModel {
                id: notesModel
                path: root.path
            }
        }

        delegate: Delegates.RoundedItemDelegate {
            id: delegateItem

            required property string name;
            required property string path;
            required property date date;
            required property int index;

            contentItem: RowLayout{
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    RowLayout {
                        id: renameLayout
                        Layout.leftMargin: 0
                        Layout.fillWidth: true
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
                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        Layout.topMargin: 7
                        Layout.bottomMargin: 7
                        text: name
                        Layout.fillWidth: true
                        elide: Qt.ElideRight
                    }
                    Label {
                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        text: Qt.formatDateTime(date, Qt.SystemLocaleDate)
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
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

                            Delegates.RoundedItemDelegate {
                                text: i18n("Rename Note")
                                icon.name: "document-edit"
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
                            Delegates.RoundedItemDelegate {
                                text: i18n("Delete Note")
                                icon.name: "delete"
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

            onClicked: {
                if (highlighted) {
                    return;
                }
                let item = pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"), {
                    path: path,
                    name: name,
                    objectName: name,
                });
            }
            highlighted: pageStack.lastItem && pageStack.lastItem.objectName === name
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
                onTriggered: noteMetadataDialog.open()
            }
        }
    }
}
