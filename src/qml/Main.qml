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
import org.kde.marknote.settings
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.statefulapp as StatetfulApp
import org.kde.kirigamiaddons.components as Components

import "components"

StatetfulApp.StatefulWindow {
    id: root
    property int minWideScreenWidth: 800
    property int normalColumnWidth: Kirigami.Units.gridUnit * 15
    property double maximalColumWidthPercentage: 0.45
    property int minimalColumnWidth: (minWideScreenWidth * maximalColumWidthPercentage) - (Kirigami.Units.gridUnit * 5)

    property bool wideScreen: applicationWindow().width >= minWideScreenWidth && !Config.fillWindow
    property bool columnModeDelayed: false

    minimumWidth: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 20

    application: App
    windowName: 'main'

    controlsVisible: false
    onWideScreenChanged: Kirigami.Settings.isMobile? drawer.close() :  (!wideScreen? (drawer.close()) : drawer.open())
    pageStack {
        globalToolBar {
            style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
            showNavigationButtons: Config.fillWindow ? Kirigami.ApplicationHeaderStyle.None : Kirigami.ApplicationHeaderStyle.ShowBackButton
        }

        columnView {
            columnResizeMode: (width >= minWideScreenWidth && !columnModeDelayed) && pageStack.depth >= 2 ? Kirigami.ColumnView.FixedColumns : Kirigami.ColumnView.SingleColumn
        }
    }

    // resizing the columns
    onWidthChanged: pageStack.defaultColumnWidth = Math.max(Math.min(root.width * maximalColumWidthPercentage, pageStack.defaultColumnWidth), minimalColumnWidth )

    property int currentWidth: normalColumnWidth

    onCurrentWidthChanged: pageStack.defaultColumnWidth = root.currentWidth

    MouseArea {
        id: collumnResizeArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        parent: applicationWindow().overlay
        visible: pageStack.columnView.columnResizeMode !== Kirigami.ColumnView.SingleColumn
        x: pageStack.defaultColumnWidth - width/2 + root.x + applicationWindow().globalDrawer.width
        width: Kirigami.Units.smallSpacing * 2
        z: root.z + 1
        cursorShape: Qt.SplitHCursor
        property int _lastX

        onPressed: mouse => {
            _lastX = mouse.x;
        }
        onPositionChanged: mouse => {
            if (_lastX == -1) {
                return;
            } else {
                const tmpWidth = Math.round(root.currentWidth - (_lastX - mouse.x));
                if (tmpWidth > minimalColumnWidth && tmpWidth < applicationWindow().width * maximalColumWidthPercentage ) root.currentWidth = tmpWidth;
            }
        }
    }


    Kirigami.Action {
        fromQAction: App.action('open_about_page')
    }

    Loader {
        id: globalMenuLoader
        active: !Kirigami.Settings.isMobile
        sourceComponent: GlobalMenuBar {}
    }

    Connections {
        target: App

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

        function onPreferences(): void {
            settingsView.open();
        }

        function onImportFromMaildir(): void {
            const component = Qt.createComponent("org.kde.marknote", "ImportMaildirDialog");
            if (component.status !== Component.Ready) {
                console.error(component.errorString());
                return;
            }
            const dialog = component.createObject(root, {
                mode: ImportMaildirDialog.Mode.Maildir,
                model: noteBooksModel,
            });
            dialog.open();
        }

        function onImportFromKNotes(): void {
            const component = Qt.createComponent("org.kde.marknote", "ImportMaildirDialog");
            if (component.status !== Component.Ready) {
                console.error(component.errorString());
                return;
            }
            const dialog = component.createObject(root, {
                mode: ImportMaildirDialog.Mode.KNotes,
                model: noteBooksModel,
            });
            dialog.open();
        }
    }

    MarkNoteSettings {
        id: settingsView

        window: root
    }

    Component.onCompleted: {
        Config.fillWindow = false;
        Kirigami.Settings.isMobile? drawer.close() :  (!wideScreen? (drawer.close()) : drawer.open())
        NavigationController.mobileMode = Kirigami.Settings.isMobile;
            if (noteBooksModel.rowCount() !== 0) {
            NavigationController.notebookPath = noteBooksModel.data(noteBooksModel.index(0, 0), NoteBooksModel.Path);
        } else {
            pageStack.push(Qt.createComponent("org.kde.marknote", "WelcomePage"), {
                model : noteBooksModel,
            });
        }
    }

    function openBottomDrawer(): void {
        bottomDrawer.open()
    }

    Connections {
        target: Kirigami.Settings

        function onIsMobileChanged(): void {
            NavigationController.mobileMode = Kirigami.Settings.isMobile;
        }
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
                    root.pageStack.push(Qt.createComponent("org.kde.marknote", "EditPage"));
                } else {
                    root.pageStack.currentIndex = root.pageStack.depth - 1;
                }
            } else {
                root.pageStack.clear();
                root.pageStack.push(Qt.createComponent("org.kde.marknote", "NotesPage"));
            }
        }
    }

    globalDrawer: Kirigami.OverlayDrawer {
        id: drawer

        Component.onCompleted: if(Config.fillWindow === true || Kirigami.Settings.isMobile === true) {
           drawer.close()
        }

        NoteBooksModel {
            id: noteBooksModel

            storagePath: Config.storage

            onNoteBookRenamed: (oldName, newName, path) => {
                if (NavigationController.notebookName === oldName) {
                    NavigationController.notebookPath = path;
                }
            }
        }

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        modal: Kirigami.Settings.isMobile ? true : false
        property double expandedWidth: 13 * Kirigami.Units.gridUnit
        property double normalWidth: 80
        width:  Config.expandedSidebar ?  expandedWidth : normalWidth
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        Behavior on width {
            NumberAnimation {
                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        contentItem: ColumnLayout {
//            visible: !Config.fillWindow
            spacing: 0

            Controls.ToolBar {
                Layout.fillWidth: true
                Layout.preferredHeight: root.pageStack.globalToolBar.preferredHeight
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
                                duration: Kirigami.Units.shortDuration * 2
                                easing.type: Easing.InOutQuart
                            }
                        }
                        Controls.Menu {
                            id: optionPopup

                            Kirigami.Action {
                                id: newNotebookAction
                                fromQAction: App.action('add_notebook')
                            }

                            Kirigami.Action {
                                id: newNoteAction
                                fromQAction: App.action('add_note')
                            }

                            Controls.Menu {
                                title: i18nc("@title:menu", "Import")
                                icon.name: "kontact-import-wizard"

                                Kirigami.Action {
                                    fromQAction: App.action('import_maildir')
                                }

                                Kirigami.Action {
                                    fromQAction: App.action('import_knotes')
                                }
                            }

                            Controls.MenuSeparator {}
                            Controls.Menu {
                                title: i18nc("@title:menu", "Sort Notes List")
                                icon.name: "view-sort"

                                Controls.MenuItem {
                                    id: sortName
                                    checkable: true
                                    text: i18n("by Name")
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
                                    text: i18n("by Date")
                                    icon.name: "view-sort-descending"
                                    autoExclusive: true
                                    onClicked: {
                                        Config.sortBehaviour = "sort-date";
                                        Config.save();
                                    }
                                    checked: Config.sortBehaviour == "sort-date"

                                }
                            }


                            Kirigami.Action {
                                fromQAction: App.action('open_kcommand_bar')
                            }

                            Kirigami.Action {
                                fromQAction: App.action('options_configure')
                            }

                            Controls.MenuSeparator {}

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
                        }
                    }

                    Kirigami.Heading {
                        text: i18nc("Application name", "Marknote")
                        horizontalAlignment: Qt.AlignHCenter
                        opacity: Config.expandedSidebar ? 1 : 0
                        width: parent.width
                        x: 0
                        y: Kirigami.Units.smallSpacing
                        Behavior on opacity {
                            NumberAnimation {
                                duration: Kirigami.Units.shortDuration * 2
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
                                duration: Kirigami.Units.shortDuration * 2
                                easing.type: Easing.InOutQuart
                            }
                        }

                    }

                }
            }

            Controls.ScrollView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                Controls.ScrollBar.vertical.interactive: false

                ListView {
                    spacing: 0
                    clip: true

                    model: noteBooksModel
                    delegate: NotebookDelegate {
                        model: noteBooksModel
                    }
                }
            }
        }
    }
    Components.BottomDrawer {
        id: bottomDrawer

        headerContentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

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
                delegate: Delegates.RoundedItemDelegate {
                    id: drawerDelegateItem

                    required property int index
                    required property string name;
                    required property string path;
                    required property string iconName;
                    required property string color;

                    Layout.fillWidth: true

                    contentItem: RowLayout {
                        Kirigami.Icon {
                            isMask: true
                            source: iconName
                            implicitHeight:Kirigami.Units.gridUnit * 1.2
                        }
                        Controls.Label { text: name}
                        Item { Layout.fillWidth: true}
                        Controls.ToolButton {
                            display: Controls.AbstractButton.IconOnly
                            action: NotebookDeleteAction {
                                path: drawerDelegateItem.path
                                name: drawerDelegateItem.name
                                model: noteBooksModel
                           }
                        }
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
