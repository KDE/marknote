// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQml.Models

import org.kde.marknote
import org.kde.notification
import org.kde.ki18n
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.treeview 1.0 as KTreeView

pragma ComponentBehavior: Bound

Kirigami.ScrollablePage {
    id: root

    readonly property bool isWideScreen: !!_window?.isWideScreen // qmllint disable missing-property

    objectName: "NotesPage"
    property color backgroundColor: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.alternateBackgroundColor, 0.6)
    background: Rectangle {color: root.backgroundColor}

    property var _window: ApplicationWindow.window
    readonly property Kirigami.PageRow pageStack: (ApplicationWindow.window as Kirigami.ApplicationWindow)?.pageStack ?? null

    readonly property string normRoot: cleanPath(NavigationController.notebookPath)
    readonly property int rootDepth: Math.max(0, (normRoot.match(/\//g) || []).length)
    property bool isTreeExpanded: true

    readonly property string activeAbsolutePath: {
        if (!NavigationController.notePath) return "";
        let rPath = root.normRoot;
        if (!rPath.endsWith("/")) rPath += "/";
        return rPath + NavigationController.notePath;
    }

    function cleanPath(p) {
        if (!p) return "";
        let s = p.toString();
        if (s.startsWith("file://")) s = s.substring(7);
            return s.replace(/\/+$/, "");
    }

    function activateItem(index, isExpandable, path) {
        if (isExpandable) {
            pageNotesModel.fetchMore(path)
            notesProxyModel.toggleChildren(index)
        } else {
            let full = cleanPath(path);
            let rPath = root.normRoot;
            if (!rPath.endsWith("/")) rPath += "/";

            if (full.startsWith(rPath)) {
                NavigationController.notePath = full.substring(rPath.length);
            }
        }
    }

    function toggleExpandAll() {
        root.isTreeExpanded = !root.isTreeExpanded;
        if (root.isTreeExpanded) {
            let i = 0;
            let limit = 0;
            // Check rowCount dynamically as the tree grows
            while (i < notesProxyModel.rowCount() && limit < 500) {
                let idx = notesProxyModel.index(i, 0);
                if (notesProxyModel.data(idx, notesProxyModel.KDescendantExpandableRole) &&
                    !notesProxyModel.data(idx, notesProxyModel.KDescendantExpandedRole)) {

                    let p = notesProxyModel.data(idx, NotesModel.Path);
                if (p) pageNotesModel.fetchMore(p);
                notesProxyModel.toggleChildren(i);
                    // Don't increment i;
                    // check the same row (now expanded) to dive into children
                    } else {
                        i++;
                    }
                    limit++;
            }
        } else {
            // Collapse backward to avoid index shifting
            for (let i = notesProxyModel.rowCount() - 1; i >= 0; i--) {
                if (notesProxyModel.data(notesProxyModel.index(i, 0), notesProxyModel.KDescendantExpandedRole)) {
                    notesProxyModel.toggleChildren(i);
                }
            }
        }
    }

    Component {
        id: noteMetadataDialogComponent
        NoteMetadataDialog {}
    }

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
            if (noteMetadataDialogComponent.status === Component.Ready) {
                const dialog = noteMetadataDialogComponent.createObject(root, {
                    mode: NoteMetadataDialog.Mode.Add,
                    model: pageNotesModel,
                }) as NoteMetadataDialog;
                dialog.open();
            } else if (noteMetadataDialogComponent.status === Component.Error) {
                console.error("Error loading NoteMetadataDialog:", noteMetadataDialogComponent.errorString());
            }
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

                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration } }
                Behavior on scale { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.OutBack } }

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
                    icon.name: root.isTreeExpanded ? "collapse-all" : "expand-all"
                    text: root.isTreeExpanded ? KI18n.i18n("Collapse All") : KI18n.i18n("Expand All")
                    display: AbstractButton.IconOnly
                    visible: root.isWideScreen

                    onClicked: root.toggleExpandAll()

                    ToolTip.visible: hovered
                    ToolTip.text: text
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
                width: titleLayout.searchOpen ? parent.width : 0
                clip: true

                Behavior on width { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic } }

                Kirigami.SearchField {
                    id: search
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: contentContainer.width
                    opacity: titleLayout.searchOpen ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration } }

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
                origin.x: searchButton.width / 2
                origin.y: searchButton.height / 2
                axis { x: 0; y: 1; z: 0 }
                angle: titleLayout.searchOpen ? 180 : 0
                Behavior on angle { NumberAnimation { duration: Kirigami.Units.veryLongDuration; easing.type: Easing.InOutQuart } }
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
            visible: root._window ? (root._window.visibility === Window.FullScreen && root.pageStack.depth !== 2) : false
            icon.name: "window-restore-symbolic"
            text: KI18n.i18nc("@action:menu", "Exit Full Screen")
            display: AbstractButton.IconOnly
            checkable: true
            checked: true
            onClicked: root._window.showNormal()

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
            pageNotesModel.deleteNote(fileUrl);
            if (notePath === NavigationController.notePath) {
                NavigationController.notePath = '';
            }
            close();
        }
        standardButtons: Dialog.Ok | Dialog.Cancel
        subtitle: KI18n.i18n("Are you sure you want to delete the note <b> %1 </b>? This will delete the file <b>%2</b> definitively.", removeDialog.noteName, removeDialog.notePath)
    }

    NotesModel {
        id: pageNotesModel
        path: NavigationController.notebookPath

        onErrorOccurred: (errorMessage) => {
            root._window.showPassiveNotification(errorMessage, "long");
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: filterModel
        property int sortOrder: Qt.AscendingOrder
        filterCaseSensitivity: Qt.CaseInsensitive
        filterRole: NotesModel.Name
        sortRole: NotesModel.Name
        sourceModel: pageNotesModel
        Component.onCompleted: filterModel.sort(0, filterModel.sortOrder)
    }

    KItemModels.KDescendantsProxyModel {
        id: notesProxyModel
        sourceModel: filterModel
    }

    ListView {
        id: notesList
        model: notesProxyModel
        anchors.fill: parent
        clip: true
        focus: true
        activeFocusOnTab: true
        boundsBehavior: Flickable.StopAtBounds

        highlightRangeMode: ListView.NoHighlightRange
        keyNavigationEnabled: true
        highlightMoveDuration: 0

        currentIndex: -1

        populate: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: Kirigami.Units.shortDuration; easing.type: Easing.OutQuart }
        }
        add: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuart }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuart }
        }
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuart }
        }
        remove: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: Kirigami.Units.longDuration; easing.type: Easing.InQuart }
        }
        removeDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuart }
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

        state: Config.sortBehaviour
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
                StateChangeScript { script: filterModel.sort(0, Qt.AscendingOrder) }
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
                StateChangeScript { script: filterModel.sort(0, Qt.DescendingOrder) }
            }
        ]

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
                    success = pageNotesModel.exportToHtml(path, selectedFile);
                } else if (selectedFile.toString().endsWith('.pdf')) {
                    success = pageNotesModel.exportToPdf(path, selectedFile);
                } else if (selectedFile.toString().endsWith('.odt')) {
                    success = pageNotesModel.exportToOdt(path, selectedFile);
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
                    text: KI18n.i18nc("@info", "Export of “%1” was successful.", exportSuccessNotification.name);
                    iconName: {
                        const ext = exportSuccessNotification.path.split('.').pop().toLowerCase();
                        switch (ext) {
                            case "pdf": return "application-pdf";
                            case "html":
                            case "htm": return "text-html";
                            case "odt": return "application-vnd.oasis.opendocument.text";
                            default: return "document-export";
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
                    text: KI18n.i18nc("@info", "Export of “%1” failed.", exportFailedNotification.name);
                    iconName: "error"
                }
            }
        }

        Components.ConvergentContextMenu {
            id: menu
            property Delegates.RoundedItemDelegate delegateItem

            Kirigami.Action {
                text: KI18n.i18nc("@action:inmenu", "Rename")
                icon.name: "document-edit"
                onTriggered: {
                    menu.delegateItem.renameField.enabled = true;
                    menu.delegateItem.renameField.forceActiveFocus();
                }
            }

            Kirigami.Action {
                text: KI18n.i18nc("@action:inmenu", "Copy")
                icon.name: "edit-copy"
                onTriggered: {
                    pageNotesModel.copyWholeNote(menu.delegateItem.fileUrl)
                    if (root._window && root.pageStack) {
                        const editorPage = root.pageStack.get(root.pageStack.depth - 1);
                        if (editorPage && editorPage.copyMessage) {
                            editorPage.copyMessage.visible = true;
                            return;
                        }
                    }
                }
            }

            Kirigami.Action {
                text: KI18n.i18nc("@action:inmenu", "Duplicate")
                icon.name: "edit-duplicate-symbolic"
                onTriggered: {
                    pageNotesModel.duplicateNote(menu.delegateItem.fileUrl)
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

            Kirigami.Action { separator: true }

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
            required property var date;
            required property int index;
            required property url fileUrl
            required property bool isFolder
            property alias renameField: renameField;

            required property int kDescendantLevel
            required property bool kDescendantExpandable
            required property bool kDescendantExpanded
            required property var kDescendantHasSiblings

            readonly property string normPath: root.cleanPath(path)
            readonly property bool isRelevant: normPath !== root.normRoot && normPath.startsWith(root.normRoot)
            readonly property int visualLevel: Math.max(1, (normPath.match(/\//g) || []).length - root.rootDepth)

            visible: isRelevant
            height: isRelevant ? implicitHeight : 0
            enabled: isRelevant

            function updateColor(): void {
                if (!delegateItem.background || !ApplicationWindow.window) {
                    return;
                }

                if (color !== '#ffffff' && color !== '#00000000') {
                    delegateItem.background.Kirigami.Theme.highlightColor = color;
                } else if (root._window) {
                    delegateItem.background.Kirigami.Theme.highlightColor = ApplicationWindow.window.Kirigami.Theme.highlightColor;
                }
            }

            onColorChanged: updateColor();
            onBackgroundChanged: updateColor();

            Component.onCompleted: {
                updateColor();
                if (kDescendantExpandable) pageNotesModel.fetchMore(path);
            }
            onPathChanged: {
                if (kDescendantExpandable) pageNotesModel.fetchMore(path);
            }
            onHighlightedChanged: {
                if (highlighted) {
                    notesList.currentIndex = delegateItem.index;
                }
            }

            Keys.onRightPressed: if (kDescendantExpandable && !kDescendantExpanded) {
                pageNotesModel.fetchMore(path);
                notesProxyModel.toggleChildren(index);
            }
            Keys.onLeftPressed: if (kDescendantExpandable && kDescendantExpanded) {
                notesProxyModel.toggleChildren(index);
            }
            Keys.onReturnPressed: root.activateItem(delegateItem.index, kDescendantExpandable, path)

            DragHandler {
                id: dragHandler
                target: null
                grabPermissions: PointerHandler.CanTakeOverFromAnything
                onActiveChanged: {
                    if (active) {
                        delegateItem.grabToImage(function(result) {
                            delegateItem.Drag.imageSource = result.url;
                            delegateItem.Drag.active = true;
                        });
                    } else {
                        delegateItem.Drag.drop();
                        delegateItem.Drag.active = false;
                    }
                }
            }

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

            ContextMenu.onRequested: (position) => {
                menu.delegateItem = delegateItem;

                if(delegateItem.kDescendantExpandable || delegateItem.isFolder) {
                    console.assert(false, "Folder menu triggered, which is yet to be implemented.")
                } else {
                    menu.popup()
                }
            }

            contentItem: RowLayout{
                spacing: Kirigami.Units.smallSpacing

                KTreeView.TreeViewDecoration {
                    Layout.fillHeight: true
                    Layout.topMargin: -delegateItem.topPadding
                    Layout.bottomMargin: -delegateItem.bottomPadding

                    model: notesProxyModel
                    parentDelegate: delegateItem
                    index: delegateItem.index

                    kDescendantLevel: delegateItem.visualLevel
                    kDescendantHasSiblings: delegateItem.kDescendantHasSiblings
                    kDescendantExpandable: delegateItem.kDescendantExpandable
                    kDescendantExpanded: delegateItem.kDescendantExpanded
                }

                Kirigami.Icon {
                    source: delegateItem.kDescendantExpandable || delegateItem.isFolder ? "folder" : "note-symbolic"
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    Layout.alignment: Qt.AlignVCenter
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

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
                                    if (renameField.text === delegateItem.name) {
                                        renameField.enabled = false;
                                        return;
                                    }

                                    pageNotesModel.renameNote(delegateItem.fileUrl, renameField.text);

                                    if (root.activeAbsolutePath === delegateItem.normPath) {
                                        let relPath = NavigationController.notePath;
                                        let lastSlash = relPath.lastIndexOf("/");

                                        if (lastSlash !== -1) {
                                            NavigationController.notePath = relPath.substring(0, lastSlash + 1) + renameField.text + '.md';
                                        } else {
                                            NavigationController.notePath = renameField.text + '.md';
                                        }
                                    }
                                    renameField.enabled = false;
                                }
                            }
                        ]
                    }

                    Label {
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        text: delegateItem.date ? Qt.formatDateTime(delegateItem.date, Qt.DefaultLocaleShortDate) : ""
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                        Layout.fillWidth: true
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        elide: Qt.ElideRight
                        visible: text !== "" && !(delegateItem.kDescendantExpandable || delegateItem.isFolder)
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

                notesList.currentIndex = delegateItem.index;

                if (delegateItem.kDescendantExpandable || delegateItem.isFolder) {
                    pageNotesModel.fetchMore(path);
                    notesProxyModel.toggleChildren(index);
                } else {
                    root.activateItem(index, false, path);
                }
            }

            highlighted: NavigationController.absoluteNotePath === delegateItem.normPath
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
