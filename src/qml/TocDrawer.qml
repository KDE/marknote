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

    required property var treeModel
    required property var blockView

    drawerOpen: false
    modal: false
    handleVisible: false
    edge: Qt.RightEdge

    rightPadding: 0

    // If the window is too narrow (i.e., less than 15 units of text + 15 units of drawer),
    // fill the whole parent width; otherwise, clip it to the editor.
    width: (parent && parent.width < (Kirigami.Units.gridUnit * 30)) ? parent.width : Kirigami.Units.gridUnit * 15

    onDrawerOpenChanged: {
        if (drawerOpen) {
            tocListView.forceActiveFocus()
        }
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.mediumSpacing

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ListView {
                id: tocListView

                topMargin: Kirigami.Units.smallSpacing
                bottomMargin: Kirigami.Units.smallSpacing
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.largeSpacing

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
                    leftPadding: ((level - 1) * Kirigami.Units.largeSpacing) + Kirigami.Units.largeSpacing
                    rightPadding: Kirigami.Units.smallSpacing

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
