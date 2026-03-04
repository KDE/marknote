// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Dialogs
import org.kde.kitemmodels
import org.kde.marknote
import org.kde.notification
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard

import "components"

Kirigami.ScrollablePage {
    id: root

    readonly property bool isWideScreen: ApplicationWindow.window ? ApplicationWindow.window.wideScreen : false

    objectName: "NotesPage"
    property color backgroundColor: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.alternateBackgroundColor, 0.6)
    background: Rectangle {color: root.backgroundColor}

    Components.FloatingButton {
        visible: Kirigami.Settings.isMobile
        parent: root.overlay
        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit
            bottomMargin: Kirigami.Units.gridUnit

        }
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
        spacing: Kirigami.Units.smallSpacing

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

            visible: root.isWideScreen
            text: NavigationController.notebookName
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            horizontalAlignment: Text.AlignHCenter
            elide: Qt.ElideRight
        }
        ToolButton {
            id: headingButton

            visible: !root.isWideScreen
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: ApplicationWindow.window.openBottomDrawer()
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
                sequences: [StandardKey.Cancel]
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
                    root.isWideScreen? heading.visible = false : headingButton.visible = false
                    addButton.visible = false
                    searchButton.icon.name = "draw-arrow-back"
                    search.forceActiveFocus()
                } else {
                    search.visible = false
                    root.isWideScreen? heading.visible = true : headingButton.visible = true
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

        ToolButton {
            visible: ApplicationWindow.window ? (ApplicationWindow.window.visibility === Window.FullScreen && ApplicationWindow.window.pageStack.depth !== 2) : false
            icon.name: "window-restore-symbolic"
            text: i18nc("@action:menu", "Exit Full Screen")
            display: AbstractButton.IconOnly
            checkable: true
            checked: true
            onClicked: ApplicationWindow.window.showNormal()

            ToolTip.text: text
            ToolTip.visible: hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    Components.MessageDialog {
        id: removeDialog

        property string notePath
        property url fileUrl
        property string noteName

        dialogType: Components.MessageDialog.Warning
        title: i18nc("@title:window", "Delete Note")
        onRejected: close()
        onAccepted: {
            notesModel.deleteNote(fileUrl);
            if (notePath === NavigationController.notePath) {
                NavigationController.notePath = '';
            }
            close();
        }
        standardButtons: Dialog.Ok | Dialog.Cancel
        subtitle: i18n("Are you sure you want to delete the note <b> %1 </b>? This will delete the file <b>%2</b> definitively.", removeDialog.noteName, removeDialog.notePath)
    }

    ListView {
        id: notesList

        enum SortRole {
            SortName,
            SortDate
        }
        currentIndex: -1

        populate: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutQuart
            }
        }

        add: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuart
            }
        }

        addDisplaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuart
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuart
            }
        }

        remove: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InQuart
            }
        }

        removeDisplaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuart
            }
        }

        Component {
            id: sectionDelegate
            Kirigami.ListSectionHeader {
                label: section
                width: parent.width
            }
        }

        Component {
            id: nullComponent

            Item {}
        }

        state: Config.sortBehaviour
        states: [
            State {
                name: "sort-name"

                PropertyChanges {
                    target: filterModel
                    sortRole: NotesModel.Name
                    sortOrder: Qt.AscendingOrder
                }
                PropertyChanges {
                    target: notesList.section
                    delegate: nullComponent
                }
                StateChangeScript {
                    script: filterModel.sort(0, Qt.AscendingOrder)
                }
            },
            State {
                name: "sort-date"

                PropertyChanges {
                    target: filterModel
                    sortRole: NotesModel.Date
                    sortOrder: Qt.DescendingOrder
                }
                PropertyChanges {
                    target: notesList.section
                    delegate: sectionDelegate
                }
                StateChangeScript {
                    script: filterModel.sort(0, Qt.DescendingOrder)
                }
            }
        ]

        model: KSortFilterProxyModel {
            id: filterModel
            property int sortOrder: Qt.AscendingOrder
            filterCaseSensitivity: Qt.CaseInsensitive
            filterRole: NotesModel.Name
            sortRole: NotesModel.Name
            sourceModel: NotesModel {
                id: notesModel
                path: NavigationController.notebookPath

                onErrorOccurred: (errorMessage) => {
                    ApplicationWindow.window.showPassiveNotification(errorMessage, "long");
                }

                onModelReset: filterModel.sort(0, filterModel.sortOrder)
            }
            Component.onCompleted: filterModel.sort(0, filterModel.sortOrder)
        }
        section {
            property: "month"
            delegate: Kirigami.ListSectionHeader {
                label: section
                width: parent.width
            }
        }

        FileDialog {
            id: fileDialog

            property string name
            property string path
            property string exportPath

            fileMode: FileDialog.SaveFile
            onAccepted: {
                var success = false;
                fileDialog.exportPath = selectedFile
                if (selectedFile.toString().endsWith('.html')) {
                    success = notesModel.exportToHtml(path, selectedFile);
                } else if (selectedFile.toString().endsWith('.pdf')) {
                    success = notesModel.exportToPdf(path, selectedFile);
                } else if (selectedFile.toString().endsWith('.odt')) {
                    success = notesModel.exportToOdt(path, selectedFile);
                }
                var notification = null;
                if (success) {
                    notification = exportSuccessNotificationComponent.createObject(this, {
                        "name": fileDialog.name,
                        "path": fileDialog.exportPath
                    });
                } else {
                    notification = exportFailedNotificationComponent.createObject(this, {
                        "name": fileDialog.name
                    });
                }

                if (notification !== null) {
                    notification.sendEvent();
                } else {
                    console.error("Failed to dynamically create the notification component.");
                }
            }

            Component {
                id: exportSuccessNotificationComponent

                Notification {
                    id: exportSuccessNotification

                    required property string path
                    required property string name

                    componentName: "marknote"
                    eventId: "exportSuccessful"
                    title: i18nc("@title:window", "Marknote")
                    text: i18nc("@info", "Export of \"%1\" was successful.", exportSuccessNotification.name);
                    iconName: {
                        const ext = exportSuccessNotification.path.split('.').pop().toLowerCase();

                        switch (ext) {
                            case "pdf":
                                return "application-pdf";
                            case "html":
                            case "htm":
                                return "text-html";
                            case "odt":
                                return "application-vnd.oasis.opendocument.text";
                            default:
                                return "document-export";
                        }
                    }
                    actions: [
                        NotificationAction {
                            label: i18nc("@action:notifaction","Open File")
                            onActivated: Qt.openUrlExternally(exportSuccessNotification.path)
                        }
                    ]
                }
            }

            Component {
                id: exportFailedNotificationComponent

                Notification {
                    id: exportFailedNotification

                    required property string name

                    componentName: "marknote"
                    eventId: "exportFailed"
                    title: i18nc("@title:window", "Marknote")
                    text: i18nc("@info", "Export of \"%1\" failed.", exportFailedNotification.name);
                    iconName: "error"
                }
            }
        }

        Components.ConvergentContextMenu {
            id: menu

            property Delegates.RoundedItemDelegate delegateItem

            Action {
                text: i18nc("@action:inmenu", "Rename Note")
                icon.name: "document-edit"
                onTriggered:
                {
                    menu.delegateItem.renameField.enabled = !menu.delegateItem.renameField.enabled

                    if(menu.delegateItem.renameField.enabled === true)
                        menu.delegateItem.renameField.forceActiveFocus()
                }
            }

            Action {
                text: i18nc("@action:inmenu", "Duplicate Note")
                icon.name: "edit-duplicate-symbolic"
                onTriggered: {
                    notesModel.duplicateNote(menu.delegateItem.fileUrl)
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
                text: i18nc("@action:inmenu", "Copy Note")
                icon.name: "edit-copy"
                onTriggered: {
                    notesModel.copyWholeNote(menu.delegateItem.fileUrl)
                    copyMessage.visible = true
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
            property alias renameField: renameField;

            function updateColor(): void {
                if (!delegateItem.background) return;

                if (color !== '#ffffff' && color !== '#00000000') {
                    delegateItem.background.Kirigami.Theme.highlightColor = color;
                } else if (ApplicationWindow.window) {
                    delegateItem.background.Kirigami.Theme.highlightColor = ApplicationWindow.window.Kirigami.Theme.highlightColor;
                }
            }

            onColorChanged: updateColor();
            onBackgroundChanged: updateColor();
            Component.onCompleted: updateColor();

            TapHandler {
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                acceptedButtons: Qt.RightButton
                onTapped: {
                    menu.delegateItem = delegateItem;
                    menu.popup()
                }
            }

            DragHandler {
                id: dragHandler
                target: null
                grabPermissions: PointerHandler.CanTakeOverFromAnything
                onActiveChanged: if (active) {
                    delegateItem.grabToImage(function(result) {
                        delegateItem.Drag.imageSource = result.url;
                    });
                }
            }

            Drag.active: dragHandler.active
            Drag.dragType: Drag.Automatic
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.proposedAction: Qt.MoveAction
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: {
                "application/x-marknote-note": fileUrl.toString(),
                "text/uri-list": fileUrl.toString()
            }

            opacity: dragHandler.active ? 0.5 : 1

            property string dragImageUrl: ""
            Drag.imageSource: dragImageUrl

            onPressed: {
                delegateItem.grabToImage(function(result) {
                    dragImageUrl = result.url;
                });
            }

            contentItem: RowLayout{
                spacing: Kirigami.Units.smallSpacing

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: Kirigami.Units.smallSpacing

                    Item{
                        id: textcolorItem
                        Kirigami.Theme.inherit: false
                        property color textcolor: Kirigami.Theme.textColor
                    }
                    Kirigami.ActionTextField {
                        id: renameField

                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing

                        // Ensure that we elide as expected, otherwise it cuts off the first half of any long title
                        autoScroll: false
                        text: name
                        onAccepted: acceptedAction.triggered();
                        visible: true
                        enabled: false
                        background.visible: enabled
                        topPadding: enabled ? Kirigami.Units.smallSpacing : 0
                        leftPadding: enabled ? Kirigami.Units.smallSpacing : 0
                        color: textcolorItem.textcolor

                        rightActions: [
                            Kirigami.Action {
                                id: acceptedAction
                                icon.name: "answer-correct"
                                enabled: renameField.text.length > 0
                                visible: renameField.enabled
                                onTriggered: {
                                    if (renameField.text.length === 0) {
                                        renameField.text = delegateItem.name;
                                    }
                                    if (renameField.text === delegateItem.name) {
                                        renameField.enabled = false
                                    }
                                    notesModel.renameNote(delegateItem.fileUrl, renameField.text);
                                    if (NavigationController.notePath === delegateItem.path) {
                                        NavigationController.notePath = renameField.text + '.md';
                                    }
                                }
                            }
                        ]
                    }

                    Label {
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        text: Qt.formatDateTime(date, Qt.SystemLocaleDate)
                        font: Kirigami.Theme.smallFont
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        elide: Qt.ElideRight

                    }
                }

                ToolButton{
                    text: i18nc("@action:button", "Show Menu")
                    icon.name: "overflow-menu"
                    down: pressed || menu.opened
                    display: ToolButton.IconOnly

                    onPressed: openMenu()

                    Keys.onReturnPressed: openMenu()
                    Keys.onEnterPressed: openMenu()

                    Layout.alignment: Qt.AlignTop
                    Layout.margins: Kirigami.Units.smallSpacing

                    Accessible.role: Accessible.ButtonMenu
                    Accessible.onPressAction: openMenu()

                    ToolTip.visible: hovered && !menu.visible
                    ToolTip.text: text
                    ToolTip.delay: Kirigami.Units.toolTipDelay

                    function openMenu(): void {
                        menu.delegateItem = delegateItem;
                        menu.popup(this, Qt.point(0, height))
                    }
                }
            }

            onClicked: {
                if (highlighted) {
                    ApplicationWindow.window.pageStack.currentIndex = ApplicationWindow.window.pageStack.depth - 1;
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
