// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kitemmodels
import org.kde.marknote
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components

import "components"

Kirigami.ScrollablePage {
    id: root

    objectName: "NotesPage"

    property bool wideScreen: applicationWindow().width >= 600

    Item{
        id: windowItem
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        property color windowBackground: Kirigami.Theme.backgroundColor
        Component.onCompleted: print(windowBackground)
    }
    Item{
        id: viewItem
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        property color viewBackground: Kirigami.Theme.backgroundColor
        Component.onCompleted: print(viewBackground)
    }
    property color backgroundColor: Kirigami.ColorUtils.linearInterpolation(windowItem.windowBackground, viewItem.viewBackground, 0.6)

    background: Rectangle {color: root.backgroundColor}

    ActionButton {
        visible: Kirigami.Settings.isMobile
        parent: root.overlay
        x: root.width - width - Kirigami.Units.gridUnit
        y: root.height - height - pageStack.globalToolBar.preferredHeight - Kirigami.Units.gridUnit
        text: i18n("Add note")
        icon.name: "list-add"
        onClicked: newNoteAction.trigger()
    }

    Connections {
        target: App
        function onNewNote(): void {
            const component = Qt.createComponent("org.kde.marknote", "NoteMetadataDialog");
            const dialog = component.createObject(root, {
                mode: NoteMetadataDialog.Mode.Add,
                model: notesModel,
            });
            dialog.open();
        }
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        ToolButton {
            id: addButton
            visible: !Kirigami.Settings.isMobile
            display: AbstractButton.IconOnly

            action: newNoteAction

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: hovered
            ToolTip.text: text
        }

        Kirigami.Heading {
            id: heading

            visible: wideScreen
            text: NavigationController.notebookName
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
                    text: NavigationController.notebookName
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
                    if (!Kirigami.Settings.isMobile) {
                        addButton.visible = true;
                    }
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

    Components.MessageDialog {
        id: removeDialog

        property string notePath
        property url fileUrl
        property string noteName

        dialogType: Components.MessageDialog.Warning
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 20)
        height: implicitHeight
        title: i18nc("@title:window", "Delete Note")
        onRejected: close()
        onAccepted: {
            notesModel.deleteNote(fileUrl);
            if (notePath === NavigationController.notePath) {
                NavigationController.notePath = '';
            }
        }
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
                path: NavigationController.notebookPath

                onErrorOccured: (errorMessage) => {
                    applicationWindow().showPassiveNotification(errorMessage, "long");
                }
            }
        }

        FileDialog {
            id: fileDialog

            property string name
            property string path

            fileMode: FileDialog.SaveFile
            onAccepted: if (selectedFile.toString().endsWith('.html')) {
                notesModel.exportToHtml(path, selectedFile);
            } else if (selectedFile.toString().endsWith('.pdf')) {
                notesModel.exportToPdf(path, selectedFile);
            } else if (selectedFile.toString().endsWith('.odt')) {
                notesModel.exportToOdt(path, selectedFile);
            }
        }

        ContextMenu {
            id: menu

            property Delegates.RoundedItemDelegate delegateItem

            visualParent: parent

            Action {
                text: i18nc("@action:inmenu", "Rename Note")
                icon.name: "document-edit"
                onTriggered: {
                    if (!menu.delegateItem.renameLayout.visible) {
                        menu.delegateItem.renameLayout.visible = true
                        menu.delegateItem.nameLabel.visible = false
                    } else {
                        menu.delegateItem.renameLayout.visible = false
                        menu.delegateItem.nameLabel.visible = true
                    }
                }
            }

            Action {
                text: i18nc("@action:inmenu", "Delete Note")
                icon.name: "delete"
                onTriggered: {
                    removeDialog.noteName = menu.delegateItem.name;
                    removeDialog.notePath = menu.delegateItem.path;
                    removeDialog.fileUrl = menu.delegateItem.fileUrl;
                    removeDialog.open()
                }
            }

            Action {
                text: i18nc("@action:inmenu", "Export to HTML")
                icon.name: "text-html"
                onTriggered: {
                    fileDialog.name = menu.delegateItem.name;
                    fileDialog.path = menu.delegateItem.fileUrl;
                    fileDialog.selectedFile = menu.delegateItem.name + '.html';
                    fileDialog.title = i18nc("@title:window", "Export to HTML");
                    fileDialog.nameFilters = [i18n("HTML file (*.html)")];
                    fileDialog.open();
                }
            }

            Action {
                text: i18nc("@action:inmenu", "Export to PDF")
                icon.name: "application-pdf"
                onTriggered: {
                    fileDialog.name = menu.delegateItem.name;
                    fileDialog.path = menu.delegateItem.fileUrl;
                    fileDialog.selectedFile = menu.delegateItem.name + '.pdf';
                    fileDialog.title = i18nc("@title:window", "Export to PDF");
                    fileDialog.nameFilters = [i18n("PDF file (*.pdf)")];
                    fileDialog.open();
                }
            }

            Action {
                text: i18nc("@action:inmenu", "Export to ODT")
                icon.name: "application-vnd.oasis.opendocument.text"
                onTriggered: {
                    fileDialog.name = menu.delegateItem.name;
                    fileDialog.path = menu.delegateItem.fileUrl;
                    fileDialog.selectedFile = menu.delegateItem.name + '.odt';
                    fileDialog.title = i18nc("@title:window", "Export to ODT");
                    fileDialog.nameFilters = [i18n("ODF Text Document (*.odt)")];
                    fileDialog.open();
                }
            }
        }

        delegate: Delegates.RoundedItemDelegate {
            id: delegateItem

            Kirigami.Theme.inherit: false
            Kirigami.Theme.backgroundColor: root.backgroundColor

            required property string name;
            required property string path;
            required property string color;
            required property date date;
            required property int index;
            required property url fileUrl
            property alias renameLayout: renameLayout;
            property alias nameLabel: nameLabel;

            function updateColor(): void {
                if (color !== '#ffffff' && color !== '#00000000') {
                    delegateItem.background.Kirigami.Theme.highlightColor = color;
                } else if (delegateItem.background.Kirigami.Theme.highlightColor !== applicationWindow().Kirigami.Theme.highlightColor) {
                    delegateItem.background.Kirigami.Theme.highlightColor = applicationWindow().Kirigami.Theme.highlightColor;
                }
            }

            onColorChanged: updateColor();
            Component.onCompleted: updateColor();

            contentItem: RowLayout{
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: Kirigami.Units.smallSpacing
                    RowLayout {
                        id: renameLayout
                        Layout.leftMargin: 0
                        Layout.fillWidth: true
                        visible: false
                        TextField {
                            Layout.fillWidth: true
                            id: renameField
                            text: name
                            onAccepted: renameButton.clicked();
                        }
                        Button {
                            id: renameButton
                            icon.name: "answer-correct"
                            enabled: renameField.text.length > 0
                            onClicked: {
                                if (renameField.text.length === 0) {
                                    renameField.text = delegateItem.name;
                                }
                                if (renameField.text === delegateItem.name) {
                                    renameLayout.visible = false;
                                    nameLabel.visible = true;
                                    return;
                                }
                                notesModel.renameNote(delegateItem.fileUrl, renameField.text);
                                if (NavigationController.notePath === delegateItem.path) {
                                    NavigationController.notePath = renameField.text + '.md';
                                }
                            }
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
                        font: Kirigami.Theme.smallFont
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        elide: Qt.ElideRight

                    }
                }

                ToolButton{
                    icon.name: "overflow-menu"

                    onClicked: {
                        menu.delegateItem = delegateItem;
                        menu.open()
                    }
                }
            }

            onClicked: {
                if (highlighted) {
                    return;
                }
                NavigationController.notePath = path;
            }
            highlighted: NavigationController.notePath === path
            topPadding: Kirigami.Units.mediumSpacing
            bottomPadding: Kirigami.Units.mediumSpacing
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            icon.name: "note"
            visible: notesList.count === 0
            text: i18n("Add a note!")
            helpfulAction: newNoteAction
        }
    }
}
