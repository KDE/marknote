/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.marknote
import org.kde.kirigamiaddons.delegates as Delegates

import "components"

Kirigami.ApplicationWindow {
    id: root
    controlsVisible: false
    property bool wideScreen: applicationWindow().width >= 600
    onWideScreenChanged: !wideScreen? drawer.close() : drawer.open()
    property string currentNotebook: noteBooksModel.rowCount() !== 0 ? noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name) : ""
    property int currentNotebookIndex: noteBooksModel.rowCount() !== 0 ? 0 : -1
    pageStack.globalToolBar.canContainHandles: wideScreen

    function openBottomDrawer() {
        bottomDrawer.open()
    }
    pageStack.globalToolBar.style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
    pageStack.popHiddenPages:true
    Component.onCompleted: if (noteBooksModel.rowCount() !== 0) {
        pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"), {
            path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
            notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name),
        });
    } else {
        pageStack.push(Qt.createComponent("org.kde.marknote", "WelcomePage"), {
            model : noteBooksModel,
        });
    }

    pageStack.defaultColumnWidth: 15 * Kirigami.Units.gridUnit

    NotebookMetadataDialog {
        id: notebookMetadataDialog

        model: noteBooksModel
    }

    globalDrawer: Kirigami.OverlayDrawer {
        id: drawer

        Shortcut {
            sequence: "Ctrl+Shift+N"
            onActivated: {
                notebookMetadataDialog.mode = NotebookMetadataDialog.Mode.Add;
                notebookMetadataDialog.open();
            }
        }

        NoteBooksModel {
            id: noteBooksModel

            onNoteBookRenamed: (oldName, newName, index) => {
                if (currentNotebook === oldName) {
                    pageStack.clear()
                    pageStack.replace([
                        Qt.createComponent("org.kde.marknote", "NotesPage"),
                        Qt.createComponent("org.kde.marknote", "EditPage")
                    ], {
                        path: noteBooksModel.data(noteBooksModel.index(index, 0), NoteBooksModel.Path),
                        notebookName: noteBooksModel.data(noteBooksModel.index(index, 0), NoteBooksModel.Name)
                    });
                }
            }
        }

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        modal: !wideScreen
        width: 80
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        contentItem: ColumnLayout {
            spacing: 0

            Controls.ToolBar {
                Layout.fillWidth: true
                Layout.preferredHeight: root.pageStack.globalToolBar.preferredHeight
                Layout.bottomMargin: Kirigami.Units.smallSpacing / 2

                contentItem: RowLayout {
                    Controls.ToolButton {
                        Layout.alignment: Qt.AlignHCenter
                        icon.name: "application-menu"
                        onClicked: optionPopup.popup()

                        Controls.Menu {
                            id: optionPopup
                            Controls.MenuItem {
                                text: i18nc("@action:inmenu", "New Notebook")
                                icon.name: "list-add-symbolic"
                                onTriggered: {
                                    notebookMetadataDialog.mode = NotebookMetadataDialog.Mode.Add;
                                    notebookMetadataDialog.open();
                                }
                            }

                            Controls.MenuItem {
                                text: i18nc("@action:inmenu", "Edit Notebook")
                                icon.name: "edit-entry-symbolic"
                                onTriggered: {
                                    notebookMetadataDialog.mode = NotebookMetadataDialog.Mode.Edit;
                                    notebookMetadataDialog.index = currentNotebookIndex;
                                    notebookMetadataDialog.name = noteBooksModel.data(noteBooksModel.index(currentNotebookIndex, 0), NoteBooksModel.Name);
                                    notebookMetadataDialog.iconName = noteBooksModel.data(noteBooksModel.index(currentNotebookIndex, 0), NoteBooksModel.Icon);
                                    notebookMetadataDialog.color = noteBooksModel.data(noteBooksModel.index(currentNotebookIndex, 0), NoteBooksModel.Color);
                                    notebookMetadataDialog.open();
                                }
                            }

                            Controls.MenuItem {
                                text: i18n("Delete Notebook")
                                icon.name: "delete"
                                onTriggered: {
                                    noteBooksModel.deleteNoteBook(currentNotebook)
                                    if(noteBooksModel.rowCount() !== 0) {
                                        pageStack.clear()
                                        pageStack.replace([
                                            Qt.createComponent("org.kde.marknote", "NotesPage"),
                                            Qt.createComponent("org.kde.marknote", "EditPage")
                                        ], {
                                            path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                                            notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
                                        })
                                    } else {
                                        pageStack.clear()
                                        pageStack.replace(Qt.createComponent("org.kde.marknote", "WelcomePage"), {
                                            model : noteBooksModel
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Repeater {
                model: noteBooksModel
                delegate: Delegates.RoundedItemDelegate {
                    id: delegateItem

                    required property int index
                    required property string name
                    required property string path
                    required property string iconName
                    required property string color

                    width: parent.width
                    icon.name: iconName
                    text: name
                    highlighted: currentNotebookIndex === index
                    contentItem: ColumnLayout {
                        Kirigami.Icon {
                            source: delegateItem.icon.name
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Controls.Label {
                            text: delegateItem.name
                            horizontalAlignment: Qt.AlignHCenter
                            elide: Text.ElideRight

                            Layout.fillWidth: true
                        }
                    }
                    onClicked: {
                        if (currentNotebook === delegateItem.name) {
                            return;
                        }
                        if (delegateItem.color !== '#000000') {
                            Kirigami.Theme.highlightColor = delegateItem.color
                            console.log(delegateItem.color)
                        }
                        currentNotebook = delegateItem.name;
                        currentNotebookIndex = delegateItem.index;
                        pageStack.clear()
                        pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"), {
                            path: delegateItem.path,
                            notebookName: delegateItem.name
                        })
                    }

                    Layout.fillWidth: true

                    Controls.ToolTip.text: text
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.delay: Kirigami.Units.toolTipDelay
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

                    required property int index
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
                                        [
                                            Qt.createComponent("org.kde.marknote", "NotesPage"),
                                            Qt.createComponent("org.kde.marknote", "EditPage")
                                        ],
                                        [{
                                            path: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path),
                                            notebookName: noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Name)
                                        }]
                                    )
                                } else {
                                    pageStack.clear()
                                    pageStack.replace(Qt.createComponent("org.kde.marknote", "WelcomePage"), {model : noteBooksModel})
                                }
                            }
                        }
                    ]
                    onClicked: {
                        bottomDrawer.close()
                        Kirigami.Theme.highlightColor = drawerDelegateItem.color
                        console.log(drawerDelegateItem.color)
                        currentNotebook = drawerDelegateItem.name
                        currentNotebookIndex = drawerDelegateItem.index;
                        pageStack.clear()
                        pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"), {
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
