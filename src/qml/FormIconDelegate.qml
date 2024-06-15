// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.iconthemes as KIconThemes
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami

FormCard.AbstractFormDelegate {
    id: root

    property alias iconName: buttonIcon.source

    text: i18nc("@action:button", "Icon")
    icon.name: "color-picker"
    onClicked: iconDialog.open()

    contentItem: RowLayout {
        spacing: 0

        Kirigami.Icon {
            source: "preferences-desktop-emoticons"
            Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
        }

        Controls.Label {
            Layout.fillWidth: true
            text: iconButton.text
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            maximumLineCount: 2
            Accessible.ignored: true // base class sets this text on root already
        }

        Kirigami.Icon {
            id: buttonIcon

            source: "addressbook-details-symbolic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        }

        FormCard.FormArrow {
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            direction: Qt.RightArrow
            visible: root.background.visible
        }
    }

    KIconThemes.IconDialog {
        id: iconDialog
        onIconNameChanged: buttonIcon.source = iconName
    }
}
