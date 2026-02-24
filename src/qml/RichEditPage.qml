// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts

import "components"

import org.kde.kirigamiaddons.components as Components
import org.kde.marknote

EditPage {
    id: richEditPage

    objectName: "RichEditPage"

    mobileToolBarHidden: mobileToolBarContainer.hidden
    mobileToolBarHeight: mobileToolBarContainer.height

    headerItems: Component {

        RowLayout{

            ToolButton {
                icon.name: "edit-undo"
                text: i18n("Undo")
                display: AbstractButton.IconOnly
                Layout.leftMargin: Kirigami.Units.smallSpacing
                onClicked: textArea.undo()
                enabled: textArea.canUndo
                visible: wideScreen && !singleDocumentMode && !mobileToolbarLayout.visible

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                icon.name: "edit-redo"
                text: i18n("Redo")
                display: AbstractButton.IconOnly
                onClicked: textArea.redo()
                enabled: textArea.canRedo
                visible: wideScreen && !singleDocumentMode && !mobileToolbarLayout.visible

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                icon.name: "view-list-details"
                text: i18nc("@action:button", "Table of Content")
                display: AbstractButton.IconOnly
                checkable: true
                checked: tocDrawer.opened
                onClicked: tocDrawer.opened ? tocDrawer.close() : tocDrawer.open()
                visible: true

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            Item {
                // for spacing
                width: Kirigami.Units.largeSpacing*5
                visible: pageStack.columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn
            }

            Item { Layout.fillWidth: true }
            Rectangle {
                height: 5
                width: height
                radius: 2.5
                scale: saved ? 0 : 1
                color: Kirigami.Theme.textColor
                Behavior on scale {
                    NumberAnimation {

                        duration: Kirigami.Units.shortDuration * 2
                        easing.type: Easing.InOutQuart
                    }
                }

            }

            Kirigami.Heading {
                text: noteName
                Layout.rightMargin: Kirigami.Units.mediumSpacing
                Layout.leftMargin: Kirigami.Units.mediumSpacing
            }

            Item{ width: 5 }

            Item { Layout.fillWidth: true }
            Item {
                width: fillWindowButton.width
                visible: wideScreen
            }


            ToolButton {
                icon.name: "search"
                text: i18nc("@action:button", "Search Note")
                display: AbstractButton.IconOnly
                visible: true
                checkable: true
                checked: searchBar.isSearchOpen
                onClicked:
                {
                    if(searchBar.isSearchOpen === true)
                    {
                        closeSearch()
                    }
                    else
                    {
                        openSearch()
                    }
                }

                ToolTip.text: i18nc("@info:tooltip", "Search in Note")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }


            ToolButton {
                id: fillWindowButton
                property int columnWidth: Config.fillWindow? 0 : Kirigami.Units.gridUnit * 15

                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: text
                ToolTip.visible: hovered
                checkable: true
                checked: Config.fillWindow
                display: AbstractButton.IconOnly
                icon.name: "view-fullscreen"
                text: i18n("Focus Mode")
                visible: wideScreen && !singleDocumentMode && !Kirigami.Settings.isMobile

                Behavior on columnWidth {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration * 2
                        easing.type: Easing.InOutQuart
                    }
                }
                onColumnWidthChanged: pageStack.defaultColumnWidth = columnWidth

                onClicked: {
                    Config.fillWindow = !Config.fillWindow
                }
                Shortcut {
                    sequence: "Ctrl+R"
                    onActivated: Config.fillWindow = !Config.fillWindow
                }
            }


            ToolButton {
                visible: richEditPage.Window.window.visibility === Window.FullScreen
                icon.name: "window-restore-symbolic"
                text: i18nc("@action:menu", "Exit Full Screen")
                display: AbstractButton.IconOnly
                checkable: true
                checked: true
                onClicked: richEditPage.Window.window.showNormal()

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }


            Button{
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: i18n("Switch editor to source mode")
                ToolTip.visible: hovered
                icon.name: "code-context-symbolic"
                checkable: true
                checked: false
                text: i18n("Source View")
                padding: 0
                flat: true
                spacing: Kirigami.Units.mediumSpacing

                onClicked: {
                    NavigationController.sourceMode = !NavigationController.sourceMode
                }
            }

        }


    }


    LinkDialog {
        id: linkDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: appwindow.window.overlay
        onAccepted: document.updateLink(linkUrl, linkText)
    }

    ImageDialog {
        id: imageDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: appwindow.window.overlay
        onAccepted: {
            if (imagePath.toString().length > 0) {
                document.insertImage(imagePath)
                imagePath = '';
            }
        }
        notePath: noteFullPath
    }

    TableDialog {
        id: tableDialog
        implicitWidth: Kirigami.Units.gridUnit * 20

        parent: appwindow.window.overlay
        onAccepted: document.insertTable(rows, cols)
    }


    Component {
        id: textFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                id: boldButton
                Shortcut {
                    sequence: StandardKey.Bold
                    onActivated: boldButton.clicked()
                }
                icon.name: "format-text-bold"
                text: i18nc("@action:button", "Bold")
                display: AbstractButton.IconOnly
                checkable: true

                checked: document.bold ?? false

                onClicked: {
                    document.bold = !document.bold
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                id: italicButton
                Shortcut {
                    sequence: StandardKey.Italic
                    onActivated: italicButton.clicked()
                }
                icon.name: "format-text-italic"
                text: i18nc("@action:button", "Italic")
                display: AbstractButton.IconOnly
                checkable: true
                checked: document.italic ?? false
                onClicked: {
                    document.italic = !document.italic;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                id: underlineButton
                Shortcut {
                    sequence: StandardKey.Underline
                    onActivated: underlineButton.clicked()
                }
                icon.name: "format-text-underline"
                text: i18nc("@action:button", "Underline")
                display: AbstractButton.IconOnly
                checkable: true
                checked: document.underline ?? false
                onClicked: {
                    document.underline = !document.underline;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                icon.name: "format-text-strikethrough"
                text: i18nc("@action:button", "Strikethrough")
                display: AbstractButton.IconOnly
                checkable: true
                checked: document.strikethrough ?? false
                onClicked: {
                    document.strikethrough = !document.strikethrough;
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }

    Kirigami.Action {
        id: indentAction

        text: i18nc("@action:button", "Increase List Level")
        icon.name: "format-indent-more"
        onTriggered: {
            document.indentListMore();
        }
        enabled: root.listIndent
    }

    Kirigami.Action {
        id: dedentAction
        icon.name: "format-indent-less"
        text: i18nc("@action:button", "Decrease List Level")
        onTriggered: {
            document.indentListLess();
        }
        enabled: root.listDedent
    }

    Component {
        id: listFormatGroup

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ToolButton {
                action: indentAction
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                action: dedentAction
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
    Component{
        id: listStyleGroup
        ComboBox {
            id: listStyleComboBox
            onActivated: (index) => {
                document.setListStyle(currentValue);
            }
            currentIndex: root.listStyle ?? 0
            enabled: indentAction.enabled || dedentAction.enabled
            textRole: "text"
            valueRole: "value"
            model: [
                { text: i18nc("@item:inmenu no list style", "No list"), value: 0 },
                { text: i18nc("@item:inmenu unordered style", "Unordered list"), value: 1 },
                { text: i18nc("@item:inmenu ordered style", "Ordered list"), value: 4 },
            ]
        }
    }
    Component{
        id: insertGroup

        RowLayout {
            ToolButton {
                id: checkboxAction
                icon.name: "checkbox-symbolic"
                text: i18nc("@action:button", "Insert checkbox")
                display: AbstractButton.IconOnly
                checkable: true
                onClicked: {
                    document.checkable = !document.checkable;
                }
                checked: checkbox
                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                id: linkAction
                icon.name: "insert-link-symbolic"
                text: i18nc("@action:button", "Insert link")
                display: AbstractButton.IconOnly
                onClicked: {
                    linkDialog.linkText = document.currentLinkText();
                    linkDialog.linkUrl = document.currentLinkUrl();
                    linkDialog.open();
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            ToolButton {
                id: imageAction
                icon.name: "insert-image-symbolic"
                text: i18nc("@action:button", "Insert image")
                display: AbstractButton.IconOnly
                onClicked: {
                    imageDialog.open();
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            ToolButton {
                id: tableAction
                icon.name: "insert-table"
                text: i18nc("@action:button", "Insert table")
                display: AbstractButton.IconOnly
                onClicked: {
                    tableDialog.open()
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

        }
    }

    Component {
        id: headingGroup
        ComboBox {
            id: headingLevelComboBox
            currentIndex: root.heading ?? 0

            model: [
                i18nc("@item:inmenu no heading", "Basic text"),
                i18nc("@item:inmenu heading level 1 (largest)", "Title"),
                i18nc("@item:inmenu heading level 2", "Subtitle"),
                i18nc("@item:inmenu heading level 3", "Section"),
                i18nc("@item:inmenu heading level 4", "Subsection"),
                i18nc("@item:inmenu heading level 5", "Paragraph"),
                i18nc("@item:inmenu heading level 6 (smallest)", "Subparagraph")
            ]

            onActivated: (index) => {
                document.setHeadingLevel(index);
            }
        }
    }

    Components.FloatingButton {
        icon.name: "document-edit"
        parent: richEditPage.overlay
        visible: !wideScreen
        scale: mobileToolBarContainer.hidden? 1 : 0

        property int defaultSpacing: Kirigami.Units.largeSpacing * 2
        property ScrollBar verticalScrollBar: contentScroll.ScrollBar.vertical

        Behavior on scale {
            NumberAnimation {

                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: verticalScrollBar.visible ?
                         defaultSpacing + verticalScrollBar.width  :
                         defaultSpacing
            bottomMargin: defaultSpacing
        }

        onClicked: mobileToolBarContainer.hidden = false

    }

    RowLayout {
        id: mobileToolBarContainer
        visible: !wideScreen
        property bool hidden: false
        y: hidden? parent.height : parent.height - mobileToolBar.height

        anchors {
            left: parent.left
            right: parent.right
        }

        z: 600000
        parent: richEditPage.overlay

        Behavior on y {
            NumberAnimation {

                duration: Kirigami.Units.shortDuration * 2
                easing.type: Easing.InOutQuart
            }
        }

        Kirigami.ShadowedRectangle {
            id: mobileToolBar

            Layout.fillHeight: true
            Layout.fillWidth: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            color: Kirigami.Theme.backgroundColor
            height: Kirigami.Units.gridUnit * 5 + Kirigami.Units.smallSpacing*2

            shadow {
                size: 15
                color: Qt.rgba(0, 0, 0, 0.2)
            }
            MouseArea {
                anchors.fill: parent
            }
            Kirigami.Separator {
                width: parent.width
                anchors.top: parent.top

            }

            ColumnLayout {
                id: mobileToolbarLayout

                anchors.fill: parent

                RowLayout {
                    SwipeView {
                        id: swipeView
                        clip: true
                        Layout.margins: Kirigami.Units.mediumSpacing
                        Layout.fillWidth: true
                        implicitHeight: undoButton.height + Kirigami.Units.smallSpacing
                        currentIndex: categorySelector.selectedIndex
                        interactive: false
                        
                        Item {
                            id: firstPage

                            RowLayout {
                                width: swipeView.width
                                height: swipeView.height
                                Loader {
                                    sourceComponent: textFormatGroup
                                    active: !wideScreen // Only active on mobile
                                }
                                Item { Layout.fillWidth: true }
                                Loader { sourceComponent: headingGroup }
                            }
                        }
                        Item {
                            id: secondPage
                            RowLayout {
                                height: swipeView.height
                                width: swipeView.width
                                Loader { sourceComponent: listFormatGroup }
                                Item { Layout.fillWidth: true }
                                Loader { sourceComponent: listStyleGroup }
                            }
                        }
                        
                        Item {
                            id: thirdPage
                            RowLayout {
                                height: swipeView.height
                                width: swipeView.width
                                Loader { sourceComponent: insertGroup }
                            }
                        }

                    }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                        Layout.topMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.mediumSpacing
                    }
                    ToolButton {
                        icon.name: "edit-undo"
                        text: i18n("Undo")
                        display: AbstractButton.IconOnly
                        onClicked: textArea.undo()
                        enabled: textArea.canUndo
                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    ToolButton {
                        id: undoButton
                        icon.name: "edit-redo"
                        text: i18n("Redo")
                        display: AbstractButton.IconOnly
                        onClicked: textArea.redo()
                        enabled: textArea.canRedo

                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                }

                RowLayout {
                    Layout.fillWidth: true

                    Item{ Layout.fillWidth: true }

                    Components.RadioSelector {
                        id: categorySelector

                        Layout.leftMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.topMargin: 0
                        Layout.fillWidth: true
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                        Layout.alignment: Qt.AlignHCenter

                        consistentWidth: true

                        actions: [
                           Kirigami.Action {
                               text: i18n("Format")
                                //icon.name: "format-border-style"
                           },
                           Kirigami.Action {
                               text: i18n("Lists")
                                //icon.name: "media-playlist-append"
                           },
                           Kirigami.Action {
                               text: i18n("Insert")
                                // icon.name: "kdenlive-add-text-clip"
                            }
                       ]
                    }

                    Item{
                        Layout.fillWidth: true
                    }

                    ToolButton {
                        icon.name: "arrow-down"
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        Layout.rightMargin: Kirigami.Units.mediumSpacing
                        icon.height: Kirigami.Units.gridUnit
                        icon.width: Kirigami.Units.gridUnit
                        Layout.alignment: Qt.AlignRight

                        Layout.topMargin: 0
                        height: categorySelector.height
                        width: height

                        onClicked: mobileToolBarContainer.hidden = true

                    }
                }
            }
        }
    }

    TocModel {
        id: tocModel
        document: textArea.textDocument
    }

    Kirigami.OverlayDrawer {
        id: tocDrawer
        edge: Qt.RightEdge
        modal: false
        handleVisible: false

        width: Kirigami.Units.gridUnit * 15
        
        parent: appwindow.window.overlay

        topMargin: (typeof pageStack !== "undefined" && pageStack.globalToolBar) ? pageStack.globalToolBar.height : (richEditPage.Window.window.header ? richEditPage.Window.window.header.height : 0)
        bottomMargin: toolBar.visible ? (toolBar.height + Kirigami.Units.largeSpacing * 2) : (mobileToolBarContainer.visible && !mobileToolBarContainer.hidden ? mobileToolBarContainer.height : 0)

        height: parent.height - topMargin - bottomMargin

        Component.onCompleted: tocDrawer.close()

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: i18nc("@title:window", "Table of Contents")
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    type: Kirigami.Heading.Type.Primary
                }

                ToolButton {
                    icon.name: "dialog-close"
                    text: i18nc("@action:button", "Close")
                    display: AbstractButton.IconOnly
                    onClicked: tocDrawer.close()

                    ToolTip.text: text
                    ToolTip.visible: hovered
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }

            ListView {
                id: tocListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: tocModel
                clip: true

                delegate: ItemDelegate {
                    id: tocDelegate
                    width: ListView.view.width

                    required property string title
                    required property int level
                    required property int index
                    required property int cursorPosition

                    text: title
                    leftPadding: (level - 1) * Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                    highlighted: ListView.isCurrentItem

                    onClicked: {
                        ListView.view.currentIndex = index
                        textArea.cursorPosition = cursorPosition
                        textArea.forceActiveFocus()
                        if (Kirigami.Settings.isMobile) {
                            tocDrawer.close()
                        }
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    icon.name: "format-list-unordered"
                    visible: tocListView.count === 0
                    text: i18n("No headers found")
                }
            }
        }
    }

    Components.FloatingToolBar {
        id: toolBar

        visible: wideScreen
        z: 600000
        parent: richEditPage.overlay

        anchors {
            bottom: parent.bottom
            margins: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        contentItem: RowLayout {
            Loader {
                sourceComponent: textFormatGroup
                active: wideScreen // Only active on desktop
            }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader { sourceComponent: listFormatGroup }
            Loader { sourceComponent: listStyleGroup }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader { sourceComponent: insertGroup }
            Kirigami.Separator {
                Layout.fillHeight: true
                Layout.margins: 0
            }
            Loader { sourceComponent: headingGroup }
        }
    }
         

    Timer {
        id: copyMessageTimer
        interval: 3000
        repeat: false
        onTriggered: copyMessage.visible = false
    }

}

