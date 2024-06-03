// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import QtQuick.Templates as T

QQC2.Dialog {
    id: root

    required property var application

    modal: true

    width: Math.min(Kirigami.Units.gridUnit * 35, parent.width)
    height: Math.min(Kirigami.Units.gridUnit * 20, parent.height)

    padding: 0

    parent: QQC2.Overlay.overlay
    anchors.centerIn: parent

    background: Components.DialogRoundedBackground {}

    onOpened: {
        searchField.forceActiveFocus();
        searchField.text = root.application.actionsModel.filterString; // set the previous searched text on reopening
        searchField.selectAll(); // select entire text
    }

    contentItem: ColumnLayout {
        spacing: 0

        Kirigami.SearchField {
            id: searchField
            onTextChanged: root.application.actionsModel.filterString = text

            Layout.fillWidth: true

            background: null

            Layout.margins: Kirigami.Units.smallSpacing

            Keys.onDownPressed: {
                listView.forceActiveFocus();
                if (listView.currentIndex < listView.count - 1) {
                    listView.currentIndex++;
                } else {
                    listView.currentIndex = 0;
                }
            }
            Keys.onUpPressed: {
                if (listView.currentIndex === 0) {
                    listView.currentIndex = listView.count - 1;
                } else {
                    listView.currentIndex--;
                }
            }
            focusSequence: ""
            autoAccept: false
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Keys.forwardTo: searchField

            ListView {
                id: actionList

                Keys.onPressed: if (event.text.length > 0) {
                    searchField.forceActiveFocus();
                    searchField.text += event.text;
                }

                clip: true

                model: root.application.actionsModel
                delegate: Delegates.RoundedItemDelegate {
                    id: commandDelegate

                    required property int index
                    required property string decoration
                    required property string displayName
                    required property string shortcut
                    required property var qaction

                    icon.name: decoration
                    text: displayName

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        Delegates.DefaultContentItem {
                            itemDelegate: commandDelegate
                            Layout.fillWidth: true
                        }

                        QQC2.Label {
                            text: commandDelegate.shortcut
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }

                    onClicked: {
                        qaction.trigger()
                        root.close()
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    text: i18n("No results found")
                    visible: actionList.count === 0
                    width: parent.width - Kirigami.Units.gridUnit * 4
                }
            }
        }
    }
}
