// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier:  GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.marknote
import org.kde.kirigamiaddons.delegates

Popup{
    id: root
    property alias filterText: sourceModel.searchText
    property alias visibleItemsCount: listView.count

    signal emojiSelected(string emojichar)

    onVisibleItemsCountChanged: {
        if (visibleItemsCount === 0 && opened){
            close()
        }
        listView.currentIndex = 0
        listView.positionViewAtIndex(0, ListView.Beginning)
    }

    onClosed: {
        listView.currentIndex = 0
        listView.positionViewAtIndex(0, ListView.Beginning)
    }
    onOpened: {
        // adding just as fallback in case onClosed one doesn't handle it
        listView.currentIndex = 0
        listView.positionViewAtIndex(0, ListView.Beginning)
    }

    function renderEmoji(code){
        if (!code){
            return ""
        }
        let codePoints = code.split("-").map(c => parseInt(c, 16))
        return String.fromCodePoint.apply(null, codePoints);
    }

    function moveSelectionUp(){
        if (listView.currentIndex !== 0){
            listView.currentIndex--;
        }
    }

    function moveSelectionDown(){
        if (listView.currentIndex !== visibleItemsCount - 1){
            listView.currentIndex++;
        }
    }

    function selectCurrent(){
        if (listView.currentItem){
            listView.currentItem.selectEmoji();
        }
    }

    EmojierProxyModel{
        id: sourceModel
    }

    padding: Kirigami.Units.smallSpacing

    contentItem: Item{
        implicitWidth: Kirigami.Units.gridUnit * 15
        implicitHeight: Kirigami.Units.gridUnit * 15

        ListView {
            id: listView
            anchors.fill: parent
            model: sourceModel
            clip: true
            currentIndex: 0

            delegate: RoundedItemDelegate{
                width: ListView.view.width
                height: Kirigami.Units.gridUnit * 2

                padding: 0
                leftInset: 0
                rightInset: 0
                topInset: 0
                bottomInset: 0

                highlighted: ListView.isCurrentItem

                contentItem: RowLayout{
                    spacing: Kirigami.Units.largeSpacing

                    Label {
                        text: root.renderEmoji(model.emojicode)
                        font.pointSize: Math.round(Kirigami.Theme.defaultFont.pointSize * 1.5)
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    }

                    Label{
                        text: ":" + model.shortcode + ":"
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    }
                }

                function selectEmoji(){
                    let emojichar = root.renderEmoji(model.emojicode);
                    root.emojiSelected(emojichar);
                }

                onClicked: {
                    selectEmoji();
                }

            }

        }
    }
}