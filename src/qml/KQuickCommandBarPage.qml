// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import QtQuick.Templates as T

Kirigami.SearchDialog {
    id: root

    required property var application

    modal: true

    parent: QQC2.Overlay.overlay
    anchors.centerIn: parent

    onTextChanged: root.application.actionsModel.filterString = text

    onAccepted: currentItem.clicked();

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

    placeholderMessage: i18n("No results found")
}
