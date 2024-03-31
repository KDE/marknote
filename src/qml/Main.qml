/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtCore
import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.marknote
import org.kde.kirigamiaddons.delegates as Delegates

import "components"

Kirigami.ApplicationWindow {
    id: root

    property bool wideScreen: applicationWindow().width >= 600

    controlsVisible: false
    onWideScreenChanged: !wideScreen? drawer.close() : drawer.open()
    pageStack {
        globalToolBar {
            canContainHandles: wideScreen
            style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
            showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
        }

        popHiddenPages:true
        defaultColumnWidth: 15 * Kirigami.Units.gridUnit
    }

    Component.onCompleted: if (noteBooksModel.rowCount() !== 0) {
        NavigationController.notebookPath = noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path);
    } else {
        pageStack.push(Qt.createComponent("org.kde.marknote", "WelcomePage"), {
            model : noteBooksModel,
        });
    }

    function openBottomDrawer() {
        bottomDrawer.open()
    }

    Kirigami.Action {
        id: newNotebookAction

        text: i18nc("@action:inmenu", "New Notebook")
        icon.name: "list-add-symbolic"
        onTriggered: {
            const component = Qt.createComponent("org.kde.marknote", "NotebookMetadataDialog");
            const dialog = component.createObject(root, {
                mode: NotebookMetadataDialog.Mode.Add,
                model: noteBooksModel,
            });
            dialog.open();
        }
    }

    Connections {
        target: NavigationController

        function onNotebookPathChanged(): void {
            if (!root.pageStack.items[0] || root.pageStack.items[0].objectName !== "NotesPage") {
                root.pageStack.clear();
                root.pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage", Component.PreferSynchronous, applicationWindow().pageStack));
            }
        }
    }

    globalDrawer: Kirigami.OverlayDrawer {
        id: drawer

        Shortcut {
            sequence: "Ctrl+Shift+N"
            onActivated: {
                newNotebookAction.trigger();
            }
        }

        NoteBooksModel {
            id: noteBooksModel

            onNoteBookRenamed: (oldName, newName, path) => {
                if (NavigationController.notebookName === oldName) {
                    NavigationController.notebookPath = path;
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
                                action: newNotebookAction
                            }
                        }
                    }
                }
            }

            Repeater {
                model: noteBooksModel
                delegate: NotebookDelegate {
                    model: noteBooksModel
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
