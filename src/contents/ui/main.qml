/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtQuick 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

import "components"

Kirigami.ApplicationWindow {
    id: root
    controlsVisible: false
    property bool wideScreen: applicationWindow().width >= 600
    onWideScreenChanged: !wideScreen? drawer.close() : drawer.open()
    property string currentNotebook: noteBooksModel.rowCount() !== 0 ? noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name) : ""
    pageStack.globalToolBar.canContainHandles: wideScreen

    function openBottomDrawer() {
        bottomDrawer.open()
    }
    pageStack.globalToolBar.style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
    pageStack.popHiddenPages:true
    Component.onCompleted: noteBooksModel.rowCount() !== 0 ? pageStack.push(
        "qrc:/contents/ui/NotesPage.qml",
        {
            path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
            notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
            }
        ): pageStack.push("qrc:/contents/ui/WelcomePage.qml", {model : noteBooksModel})


    pageStack.defaultColumnWidth: 15 * Kirigami.Units.gridUnit

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer

        Shortcut {
            sequence: "Ctrl+Shift+N"
            onActivated: addNotebookDialog.open()
        }
        NoteBooksModel {
            id: noteBooksModel
        }

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        modal: !wideScreen
        width: 60
        margins: 0
        padding: 0
        header: Kirigami.AbstractApplicationHeader {
            RowLayout {
                anchors.fill: parent
                Controls.ToolButton {
                    Layout.alignment: Qt.AlignHCenter
                    icon.name: "application-menu"
                    onClicked: optionPopup.popup()

                    AddNotebookDialog {
                        id: addNotebookDialog
                        model: noteBooksModel
                    }
                    Controls.Menu {
                        id: optionPopup
                        Controls.MenuItem {
                            text: i18n("New Notebook")
                            icon.name: "list-add"
                            onTriggered: { addNotebookDialog.open() }

                        }
//                        Controls.MenuItem {
//                            text: i18n("Edit Notebook")
//                            icon.name: "edit-entry"

//                        }
                        Controls.MenuItem {
                            text: i18n("Delete Notebook")
                            icon.name: "delete"
                            onTriggered: {
                                noteBooksModel.deleteNoteBook(currentNotebook)
                                if(noteBooksModel.rowCount() !== 0) {
                                    pageStack.clear()
                                    pageStack.replace(["qrc:/contents/ui/NotesPage.qml","qrc:/contents/ui/EditPage.qml"], {
                                        path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                                        notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
                                    })
                                } else {
                                    pageStack.clear()
                                    pageStack.replace("qrc:/contents/ui/WelcomePage.qml", {model : noteBooksModel})
                                }
                            }
                        }
                    }
                }
            }
        }
        ColumnLayout {
            Repeater {
                model: noteBooksModel
                delegate: Kirigami.NavigationTabButton {
                    id: delegateItem
                    required property string name;
                    required property string path;
                    required property string iconName;
                    required property string color;
                    Kirigami.Theme.highlightColor: delegateItem.color
                    Layout.fillWidth: true
                    implicitHeight: 50
                    icon.name: iconName
                    text: name
                    Layout.margins: 0
                    onClicked: {
                        Kirigami.Theme.highlightColor = delegateItem.color
                        console.log(delegateItem.color)
                        currentNotebook = delegateItem.name
                        pageStack.clear()
                        pageStack.push("qrc:/contents/ui/NotesPage.qml", {
                            path: delegateItem.path,
                            notebookName: delegateItem.name

                            }
                        )
                    }
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
    BottomDrawer {
        id: bottomDrawer

        headerContentItem: RowLayout {
            Kirigami.Heading {
                text: i18n("Your Notebooks")
            }
            Item { Layout.fillWidth: true }
            Controls.ToolButton {
                icon.name: "list-add"
                onClicked: {
                    addNotebookDialog.open()
                    bottomDrawer.close()
                }
            }
        }
        drawerContentItem: ColumnLayout {
            Repeater {
                model: noteBooksModel
                delegate: Kirigami.SwipeListItem {
                    id: drawerDelegateItem
                    required property string name;
                    required property string path;
                    required property string iconName;
                    required property string color;
                    Layout.fillWidth: true
                    alwaysVisibleActions:true

                    contentItem: RowLayout {
                        Kirigami.Icon {
                            isMask: true
                            source: iconName
                            implicitHeight:Kirigami.Units.gridUnit * 1.2
                        }
                        Controls.Label { text: name}
                        Item { Layout.fillWidth: true}
                    }

                    actions: [
                        Kirigami.Action {
                            icon.name: "delete"
                            text: i18n("Delete Notebook")
                            onTriggered: {
                                noteBooksModel.deleteNoteBook(drawerDelegateItem.name)
                                if(noteBooksModel.rowCount() !== 0) {
                                    pageStack.clear()
                                    pageStack.replace(
                                        ["qrc:/contents/ui/NotesPage.qml","qrc:/contents/ui/EditPage.qml"],
                                        {
                                        path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                                        notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
                                        }
                                    )
                                } else {
                                    pageStack.clear()
                                    pageStack.replace("qrc:/contents/ui/WelcomePage.qml", {model : noteBooksModel})
                                }
                            }
                        }
                    ]
                    onClicked: {
                        bottomDrawer.close()
                        Kirigami.Theme.highlightColor = drawerDelegateItem.color
                        console.log(drawerDelegateItem.color)
                        currentNotebook = drawerDelegateItem.name
                        pageStack.clear()
                        pageStack.push("qrc:/contents/ui/NotesPage.qml", {
                            path: drawerDelegateItem.path,
                            notebookName: drawerDelegateItem.name

                            }
                        )
                    }
                }
            }
            Item { height: Kirigami.Units.largeSpacing * 3}
        }

    }
}
