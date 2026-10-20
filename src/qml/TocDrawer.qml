// SPDX-FileCopyrightText: 2026 Shubham Shinde <shindeshubham0520@gmail.com>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier:  GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigami as Kirigami
import org.kde.marknote
import org.kde.ki18n

Kirigami.OverlayDrawer {
    id: root
    HoverHandler {
        id: tocArea
        enabled: true
        onHoveredChanged: {
            if (!hovered && !pinned) {
                root.close()
            }
        }
    }
    required property var treeModel
    required property var blockView
    property bool pinned: false

    drawerOpen: false
    modal: false
    handleVisible: false
    edge: Qt.RightEdge


    // If the window is too narrow (i.e., less than 15 units of text + 15 units of drawer),
    // fill the whole parent width; otherwise, clip it to the editor.
    width: (parent && parent.width < (Kirigami.Units.gridUnit * 30)) ? parent.width : Kirigami.Units.gridUnit * 15
    opacity: 0
    onAboutToShow: opacity = 1
    onAboutToHide: opacity = 0

    Behavior on opacity {
        NumberAnimation {
            duration: Kirigami.Units.longDuration * 2
            easing.type: Easing.InOutQuad
        }
    }

    background: Item{}
    onDrawerOpenChanged: {
        if (drawerOpen) {
            tocListView.forceActiveFocus()
        }
    }

    contentItem: ColumnLayout {
        id: contentItem
        spacing: Kirigami.Units.smallSpacing



        ScrollView {
            id: tocScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.rightMargin: Kirigami.Units.gridUnit * 2
            Layout.topMargin: Kirigami.Units.gridUnit
            Layout.bottomMargin: Kirigami.Units.gridUnit * 4

            rightPadding: Kirigami.Units.largeSpacing


            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            background: Item{
                Kirigami.ShadowedRectangle {
                    id: rec

                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    Kirigami.Theme.inherit: false
                    color.r: Kirigami.Theme.backgroundColor.r
                    color.g: Kirigami.Theme.backgroundColor.g
                    color.b: Kirigami.Theme.backgroundColor.b
                    color.a: root.pinned ? 0 : 1
                    radius: Kirigami.Units.cornerRadius *2
                    Behavior on color.a {
                        NumberAnimation {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }
                    }
                    shadow {
                        size: root.pinned ? 0 : 15
                        yOffset: root.pinned ? 0 : 3
                        color: Qt.rgba(0, 0, 0, 0.2)

                        Behavior on size {
                            NumberAnimation {
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }


                        Behavior on yOffset {
                            NumberAnimation {
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }

                    border {
                        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
                        width: 1
                    }
                    width: tocScrollView.width
                    height: Math.min(tocScrollView.height,tocScrollView.contentHeight + 2 * Kirigami.Units.largeSpacing)
                    // anchors.top: parent.top
                    anchors.horizontalCenter: tocListView.horizontalCenter
                }
            }
            ListView {
                id: tocListView

                header: RowLayout {
                    width: parent.width
                    Kirigami.Heading {
                        text: KI18n.i18nc("@title:window", "Table of Contents")
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        type: Kirigami.Heading.Type.Primary
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing

                    }

                    ToolButton {
                        icon.name: "pin"
                        text: KI18n.i18nc("@action:button", "pin")
                        display: AbstractButton.IconOnly
                        checkable: true

                        onClicked: root.pinned = !root.pinned
                        checked: root.pinned
                        ToolTip.text: text
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 0
                        Layout.bottomMargin: Kirigami.Units.largeSpacing

                    }
                }
                topMargin: Kirigami.Units.largeSpacing
                leftMargin: Kirigami.Units.smallSpacing
                bottomMargin: Kirigami.Units.largeSpacing


                clip: true
                focus: true

                model: TocModel {
                    id: tocModel
                    treeModel: root.treeModel
                }

                onCountChanged: {
                    if (count > 0 && root.blockView) {
                        let activeIndex = tocModel.headingIndexAtBlock(root.blockView.currentIndex)
                        currentIndex = activeIndex !== -1 ? activeIndex : 0
                    }
                }

                onCurrentIndexChanged: {
                    positionViewAtIndex(currentIndex, ListView.Contain)
                }

                delegate: Delegates.RoundedItemDelegate {
                    id: tocDelegate

                    width: ListView.view.width

                    required property string title
                    required property int level
                    required property int index
                    required property int blockIndex

                    text: title
                    leftPadding: ((level - 1) * 1.5 * Kirigami.Units.largeSpacing) + Kirigami.Units.largeSpacing
                    rightPadding: Kirigami.Units.largeSpacing

                    topPadding: Kirigami.Units.smallSpacing
                    bottomPadding: Kirigami.Units.smallSpacing

                    highlighted: ListView.isCurrentItem

                    onClicked: {
                        ListView.view.currentIndex = index
                        if (root.blockView) {
                            root.blockView.currentIndex = blockIndex
                            root.blockView.positionViewAtIndex(blockIndex, ListView.Beginning)
                            root.blockView.forceActiveFocus()
                        }

                        // Dismiss the drawer on narrow screens so they can read
                        if (root.width === (root.parent ? root.parent.width : 0)) {
                            root.close()
                        }
                    }
                }

                Connections {
                    target: root.blockView
                    function onCurrentIndexChanged() {
                        if (!root.blockView) return;
                        let activeIndex = tocModel.headingIndexAtBlock(root.blockView.currentIndex)

                        if (activeIndex !== -1 && activeIndex !== tocListView.currentIndex) {
                            tocListView.currentIndex = activeIndex
                        }
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    icon.name: "format-list-unordered"
                    visible: tocListView.count === 0
                    text: KI18n.i18nc("Is shown instead of the table of contents", "No headings found")
                }
            }
        }
    }
}
