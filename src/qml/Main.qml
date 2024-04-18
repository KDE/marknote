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

    minimumWidth: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 20

    controlsVisible: false
    onWideScreenChanged: !wideScreen? drawer.close() : drawer.open()
    pageStack {
        globalToolBar {
            style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
            showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
        }

        defaultColumnWidth: 15 * Kirigami.Units.gridUnit
        columnView {
            columnResizeMode: width >= pageStack.defaultColumnWidth * 3.5 && pageStack.depth >= 2 ? Kirigami.ColumnView.FixedColumns : Kirigami.ColumnView.SingleColumn
        }
    }

    Loader {
        id: kcommandbarLoader
        active: false
        sourceComponent: KQuickCommandBarPage {
            application: App
            onClosed: kcommandbarLoader.active = false
        }
        onActiveChanged: if (active) {
            item.open()
        }
    }

    Connections {
        target: App

        function onOpenKCommandBarAction(): void {
            kcommandbarLoader.active = true;
        }

        function onNewNotebook(): void {
            const component = Qt.createComponent("org.kde.marknote", "NotebookMetadataDialog");
            const dialog = component.createObject(root, {
                mode: NotebookMetadataDialog.Mode.Add,
                model: noteBooksModel,
            });
            dialog.open();
        }

        function onNewNote(): void {
            if (NavigationController.notebookPath.length === 0) {
                root.showPassiveNotification(i18nc("@info:status", "Unable to create a new note, you need to create a notebook first."), "long", i18nc("@action:button", "Create Notebook"), () => {
                    newNotebookAction.trigger();
                });
                return;
            }
        }

        function onOpenAboutPage(): void {
            const openDialogWindow = pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }

        function onOpenAboutKDEPage(): void {
            const openDialogWindow = pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDE"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }
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

    Connections {
        target: NavigationController

        function onNotebookPathChanged(): void {
            if (!root.pageStack.items[0] || root.pageStack.items[0].objectName !== "NotesPage") {
                root.pageStack.clear();
                root.pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"));
            }
        }

        function onNotePathChanged(): void {
            if (NavigationController.notePath.length > 0) {
                if (!root.pageStack.items[1] || root.pageStack.items[1].objectName !== "EditPage") {
                    root.pageStack.clear();
                    root.pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"));
                    if (!Kirigami.Settings.isMobile) {
                        root.pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"));
                    }
                }
            } else {
                root.pageStack.clear();
                root.pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"));
            }
        }
    }

    globalDrawer: Kirigami.OverlayDrawer {
        id: drawer

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
        property double expandedWidth: 13 * Kirigami.Units.gridUnit
        property double normalWidth: 80
        width: Config.expandedSidebar ?  expandedWidth : normalWidth
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        Behavior on width {
            NumberAnimation {
                duration: Kirigami.Units.mediumDuration
                easing.type: Easing.InOutQuart
            }
        }

        contentItem: ColumnLayout {
            spacing: 0

            Controls.ToolBar {
                Layout.fillWidth: true
                Layout.preferredHeight: root.pageStack.globalToolBar.preferredHeight
                Layout.bottomMargin: Kirigami.Units.smallSpacing / 2
                leftPadding: 0
                rightPadding: 0


                contentItem: Item {
                    Controls.ToolButton {
                        id: menuButton
                        icon.name: "application-menu"
                        onClicked: optionPopup.popup()
                        x: Config.expandedSidebar ? Kirigami.Units.smallSpacing : drawer.normalWidth / 2 - width / 2

                        Behavior on x {
                            NumberAnimation {
                                duration: Kirigami.Units.mediumDuration
                                easing.type: Easing.InOutQuart
                            }
                        }
                        Controls.Menu {
                            id: optionPopup

                            Controls.MenuItem {
                                action: KActionFromAction {
                                    id: newNotebookAction
                                    action: App.action('add_notebook')
                                }
                            }

                            Controls.MenuItem {
                                action: KActionFromAction {
                                    id: newNoteAction
                                    action: App.action('add_note')
                                }
                            }

                            Controls.MenuSeparator {}

                            Controls.MenuItem {
                                id: sortName
                                checkable: true
                                text: i18n("Sort by Name")
                                icon.name: "sort-name"
                                autoExclusive: true
                                onClicked: {
                                    Config.sortBehaviour = "sort-name";
                                    Config.save();
                                }
                                checked: Config.sortBehaviour == "sort-name"

                            }

                            Controls.MenuItem {
                                id: sortDate
                                checkable: true
                                text: i18n("Sort by Date")
                                icon.name: "view-sort-descending"
                                autoExclusive: true
                                onClicked: {
                                    Config.sortBehaviour = "sort-date";
                                    Config.save();
                                }
                                checked: Config.sortBehaviour == "sort-date"

                            }

                            Controls.MenuSeparator {}

                            Controls.MenuItem {
                                action: KActionFromAction {
                                    action: App.action('open_kcommand_bar')
                                }
                            }

                            Controls.MenuItem {
                                id: expandSidebar
                                text: Config.expandedSidebar ? i18n("Collapse Sidebar") : i18n("Expand Sidebar")
                                icon.name: Config.expandedSidebar ? "sidebar-collapse-left" : "sidebar-expand-left"
                                onClicked: {
                                    Config.expandedSidebar = !Config.expandedSidebar;
                                    Config.save();
                                }
                                Shortcut {
                                    sequence: "Ctrl+Shift+S"
                                    onActivated: expandSidebar.clicked()
                                }
                            }
                            Controls.MenuItem {
                                action: KActionFromAction {
                                    action: App.action('open_about_page')
                                }
                            }

                            Controls.MenuItem {
                                text: i18nc("@action:inmenu", "Settings")
                                onClicked: {
                                    const component = Qt.createComponent("org.kde.marknote", "SettingsDialog");
                                    if (component.status !== Component.Ready) {
                                        console.error(component.errorString());
                                        return;
                                    }
                                    const dialog = component.createObject(root);
                                    dialog.open();
                                }
                            }
                        }
                    }
                    Kirigami.Heading {
                        text: i18n("MarkNote")
                        horizontalAlignment: Qt.AlignHCenter
                        opacity: Config.expandedSidebar ? 1 : 0
                        width: parent.width
                        x: 0
                        y: Kirigami.Units.smallSpacing
                        Behavior on opacity {
                            NumberAnimation {
                                duration: Kirigami.Units.mediumDuration
                                easing.type: Easing.InOutQuart
                            }
                        }
                    }
                    Controls.ToolButton {
                        icon.name: "sidebar-collapse-left"
                        onClicked: expandSidebar.clicked()
                        x: drawer.width - width - Kirigami.Units.smallSpacing
                        opacity: Config.expandedSidebar ? 1 : 0
                        enabled: Config.expandedSidebar
                        Behavior on opacity {
                            NumberAnimation {
                                duration: Kirigami.Units.mediumDuration
                                easing.type: Easing.InOutQuart
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
                    newNotebookAction.trigger();
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

                    actions: NotebookDeleteAction {
                        path: drawerDelegateItem.path
                        name: drawerDelegateItem.name
                        model: noteBooksModel
                    }

                    onClicked: {
                        bottomDrawer.close()
                        Kirigami.Theme.highlightColor = drawerDelegateItem.color
                        NavigationController.notebookPath = drawerDelegateItem.path
                        pageStack.clear()
                        pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"));
                    }
                }
            }
            Item { height: Kirigami.Units.largeSpacing * 3}
        }
    }
}
