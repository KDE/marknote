/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

Kirigami.ApplicationWindow {
    id: root

    title: i18n("marknote")

    pageStack.globalToolBar.style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
    pageStack.initialPage: ["qrc:/NotesPage.qml","qrc:/EditPage.qml"]

    pageStack.defaultColumnWidth: 15 * Kirigami.Units.gridUnit

    globalDrawer: Kirigami.GlobalDrawer {
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
                            text: "Add new Notebook"
                            icon.name: "list-add"
                            onTriggered: { addNotebookDialog.open() }

                        }
                        Controls.MenuItem {
                            text: "Edit Notebook"
                            icon.name: "edit-entry"

                        }
                        Controls.MenuItem {
                            text: "Delete Notebook"
                            icon.name: "delete"

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

                    Layout.fillWidth: true
                    implicitHeight: 50
                    icon.name: "addressbook-details"
                    text: name
                    Layout.margins: 0
                    onClicked: {
                        pageStack.clear()
                        pageStack.push(["qrc:/NotesPage.qml","qrc:/EditPage.qml"], {
                            path: delegateItem.path
                        }

                        )
                    }

                }

            }
            Item { Layout.fillHeight: true }
        }
    }
}
