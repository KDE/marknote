// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.kitemmodels
import org.kde.marknote
import org.kde.notification
import org.kde.ki18n
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components

pragma ComponentBehavior: Bound

Kirigami.ScrollablePage {
    id: root

    readonly property bool isWideScreen: !!ApplicationWindow.window?.isWideScreen // qmllint disable missing-property

    objectName: "NotesPage"
    property color backgroundColor: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.alternateBackgroundColor, 0.6)
    background: Rectangle {color: root.backgroundColor}

    property var _window: ApplicationWindow.window
    readonly property Kirigami.PageRow pageStack: (ApplicationWindow.window as Kirigami.ApplicationWindow)?.pageStack ?? null

    Components.FloatingButton {
        visible: Kirigami.Settings.isMobile
        parent: root.overlay
        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit
            bottomMargin: Kirigami.Units.gridUnit

        }
        text: KI18n.i18n("Add note")
        icon.name: "list-add"
        onClicked: newNoteAction.trigger()
    }

    enum SortRole {
        SortName,
        SortDate
    }

    Connections {
        target: App
        function onNewNote(): void {
            const component = Qt.createComponent("org.kde.marknote", "NoteMetadataDialog");
            const dialog = component.createObject(root, {
                mode: NoteMetadataDialog.Mode.Add,
                model: notesModel,
            }) as NoteMetadataDialog;
            dialog.open();
        }
    }

    titleDelegate: RowLayout {
        id: titleLayout
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        property bool searchOpen: false

        Item {
            id: contentContainer
            Layout.fillWidth: true
            implicitHeight: Math.max(defaultHeader.implicitHeight, search.implicitHeight)

            RowLayout {
                id: defaultHeader
                anchors.fill: parent

                scale: titleLayout.searchOpen ? 0.7 : 1.0
                opacity: titleLayout.searchOpen ? 0 : 1
                visible: opacity > 0

                transformOrigin: Item.Center

                Behavior on opacity {
                    NumberAnimation { duration: Kirigami.Units.longDuration }
                }

                // Animate the scale with a bouncy easing curve
                Behavior on scale {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutBack
                    }
                }

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
                    contentItem: RowLayout {
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
            }

            Item {
                id: searchWrapper
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                // Animate this wrapper's width to "uncover" the search bar
                width: titleLayout.searchOpen ? parent.width : 0
                clip: true

                Behavior on width {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutCubic
                    }
                }

                Kirigami.SearchField {
                    id: search
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom

                    // keep width static so the placeholder text doesn't squish during animation
                    width: contentContainer.width

                    opacity: titleLayout.searchOpen ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation { duration: Kirigami.Units.longDuration }
                    }

                    Shortcut {
                        id: cancelShortcut
                        sequences: [StandardKey.Cancel]
                        onActivated: if (titleLayout.searchOpen) { searchButton.clicked() }
                    }
                    onTextChanged: filterModel.setFilterFixedString(search.text)
                }
            }
        }

        ToolButton {
            id: searchButton
            icon.name: titleLayout.searchOpen ? "draw-arrow-back" : "search"
            text: titleLayout.searchOpen ? KI18n.i18n("Exit Search (%1)", cancelShortcut.nativeText) : KI18n.i18n("Search notes (%1)", searchShortcut.nativeText)
            display: AbstractButton.IconOnly

            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: hovered
            ToolTip.text: text

            transform: Rotation {
                // Set the origin to the center of the button so it spins in place
                origin.x: searchButton.width / 2
                origin.y: searchButton.height / 2

                axis { x: 0; y: 1; z: 0 }

                // Spin a half 180 degrees so the new icon lands facing the right way
                angle: titleLayout.searchOpen ? 180 : 0

                Behavior on angle {
                    NumberAnimation {
                        duration: Kirigami.Units.veryLongDuration
                        easing.type: Easing.InOutQuart
                    }
                }
            }

            onClicked: {
                titleLayout.searchOpen = !titleLayout.searchOpen;
                if (titleLayout.searchOpen) {
                    search.forceActiveFocus();
                } else {
                    search.clear();
                }
            }

            Shortcut {
                id: searchShortcut
                sequence: "Ctrl+E"
                onActivated: if (!titleLayout.searchOpen) {
                    searchButton.clicked()
                }
            }
        }

        ToolButton {
            visible: ApplicationWindow.window ? (ApplicationWindow.window.visibility === Window.FullScreen && root.pageStack.depth !== 2) : false
            icon.name: "window-restore-symbolic"
            text: KI18n.i18nc("@action:menu", "Exit Full Screen")
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
        title: KI18n.i18nc("@title:window", "Delete Note")
        onRejected: close()
        onAccepted: {
            notesModel.deleteNote(fileUrl);
            if (notePath === NavigationController.notePath) {
                NavigationController.notePath = '';
            }
            close();
        }
        standardButtons: Dialog.Ok | Dialog.Cancel
        subtitle: KI18n.i18n("Are you sure you want to delete the note <b> %1 </b>? This will delete the file <b>%2</b> definitively.", removeDialog.noteName, removeDialog.notePath)
    }

    ListView {
        id: notesList

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
                required property string section

                text: section
                width: parent.width
            }
        }

        Component {
            id: nullComponent

            Item {}
        }

        states: [
            State {
                name: "sort-name"

                PropertyChanges {
                    filterModel.sortRole: NotesModel.Name
                    filterModel.sortOrder: Qt.AscendingOrder
                }
                PropertyChanges {
                    notesList.section.delegate: nullComponent
                }
                StateChangeScript {
                    script: filterModel.sort(0, Qt.AscendingOrder)
                }
            },
            State {
                name: "sort-date"

                PropertyChanges {
                    filterModel.sortRole: NotesModel.Date
                    filterModel.sortOrder: Qt.DescendingOrder
                }
                PropertyChanges {
                    notesList.section.delegate: sectionDelegate
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
                required property string section

                text: section
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
                    title: KI18n.i18nc("@title:window", "Marknote")
                    text: KI18n.i18nc("@info", "Export of \"%1\" was successful.", exportSuccessNotification.name);
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
                            label: KI18n.i18nc("@action:notifaction","Open File")
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
                    title: KI18n.i18nc("@title:window", "Marknote")
                    text: KI18n.i18nc("@info", "Export of \"%1\" failed.", exportFailedNotification.name);
                    iconName: "error"
                }
            }
        }

        Components.ConvergentContextMenu {
            id: menu

            property Delegates.RoundedItemDelegate delegateItem

            Action {
                text: KI18n.i18nc("@action:inmenu", "Rename")
                icon.name: "document-edit"
                onTriggered:
                {
                    menu.delegateItem.renameField.enabled = !menu.delegateItem.renameField.enabled

                    if(menu.delegateItem.renameField.enabled === true)
                        menu.delegateItem.renameField.forceActiveFocus()
                }
            }

            Action {
                text: KI18n.i18nc("@action:inmenu", "Copy")
                icon.name: "edit-copy"
                onTriggered: {
                    notesModel.copyWholeNote(menu.delegateItem.fileUrl)

                    if (root._window && root.pageStack) {
                        const editorPage = root.pageStack.get(root.pageStack.depth - 1);

                        if (editorPage && editorPage.copyMessage) {
                            editorPage.copyMessage.visible = true;
                            return;
                        }
                    }
                }
            }

            Action {
                text: KI18n.i18nc("@action:inmenu", "Duplicate")
                icon.name: "edit-duplicate-symbolic"
                onTriggered: {
                    notesModel.duplicateNote(menu.delegateItem.fileUrl)
                }
            }

            Kirigami.Action {
                text: KI18n.i18nc("@action:inmenu", "Export")
                icon.name: "document-export"

                Kirigami.Action {
                    text: KI18n.i18nc("@action:inmenu", "Export to HTML")
                    icon.name: "text-html"
                    onTriggered: {
                        fileDialog.name = menu.delegateItem.name;
                        fileDialog.path = menu.delegateItem.fileUrl;
                        fileDialog.selectedFile = menu.delegateItem.name + '.html';
                        fileDialog.title = KI18n.i18nc("@title:window", "Export to HTML");
                        fileDialog.nameFilters = [KI18n.i18n("HTML file (*.html)")];
                        fileDialog.open();
                    }
                }

                Kirigami.Action {
                    text: KI18n.i18nc("@action:inmenu", "Export to PDF")
                    icon.name: "application-pdf"
                    onTriggered: {
                        fileDialog.name = menu.delegateItem.name;
                        fileDialog.path = menu.delegateItem.fileUrl;
                        fileDialog.selectedFile = menu.delegateItem.name + '.pdf';
                        fileDialog.title = KI18n.i18nc("@title:window", "Export to PDF");
                        fileDialog.nameFilters = [KI18n.i18n("PDF file (*.pdf)")];
                        fileDialog.open();
                    }
                }

                Kirigami.Action {
                    text: KI18n.i18nc("@action:inmenu", "Export to ODT")
                    icon.name: "application-vnd.oasis.opendocument.text"
                    onTriggered: {
                        fileDialog.name = menu.delegateItem.name;
                        fileDialog.path = menu.delegateItem.fileUrl;
                        fileDialog.selectedFile = menu.delegateItem.name + '.odt';
                        fileDialog.title = KI18n.i18nc("@title:window", "Export to ODT");
                        fileDialog.nameFilters = [KI18n.i18n("ODF Text Document (*.odt)")];
                        fileDialog.open();
                    }
                }
            }

            Kirigami.Action {
                separator: true
            }

            Action {
                text: KI18n.i18nc("@action:inmenu", "Delete")
                icon.name: "delete"
                onTriggered: {
                    removeDialog.noteName = menu.delegateItem.name;
                    removeDialog.notePath = menu.delegateItem.path;
                    removeDialog.fileUrl = menu.delegateItem.fileUrl;
                    removeDialog.open()
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

                    Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing

                        Layout.preferredHeight: renameField.implicitHeight
                        verticalAlignment: Text.AlignVCenter

                        text: delegateItem.name
                        elide: Qt.ElideRight
                        visible: !renameField.enabled
                        color: textcolorItem.textcolor
                    }

                    Kirigami.ActionTextField {
                        id: renameField

                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing

                        text: delegateItem.name
                        onAccepted: acceptedAction.triggered();

                        visible: enabled
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
                        text: Qt.formatDateTime(delegateItem.date)
                        font: Kirigami.Theme.smallFont
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        elide: Qt.ElideRight

                    }
                }

                ToolButton{
                    text: KI18n.i18nc("@action:button", "Show Menu")
                    icon.name: "overflow-menu"
                    down: pressed || (menu.opened && menu.delegateItem === delegateItem)
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
                    root.pageStack.currentIndex = root.pageStack.depth - 1;
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
            text: KI18n.i18n("Add a note!")
            helpfulAction: newNoteAction
        }
    }



}
