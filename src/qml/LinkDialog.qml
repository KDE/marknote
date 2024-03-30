// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard

QQC2.Dialog {
    id: root

    property alias linkText: linkTextField.text
    property alias linkUrl: linkUrlField.text

    x: Math.round((parent.width - width) / 2)
    y: Math.round(parent.height / 3)

    width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 15)

    title: i18nc("@title:window", "Insert Link")

    background: Components.DialogRoundedBackground {}

    modal: true
    focus: true

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    header: Kirigami.Heading {
        text: root.title
        elide: QQC2.Label.ElideRight
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing
    }

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormTextFieldDelegate {
            id: linkTextField

            label: i18nc("@label:textbox", "Link Text:")
            leftPadding: Kirigami.Units.largeSpacing * 2
            rightPadding: Kirigami.Units.largeSpacing * 2
        }

        FormCard.FormDelegateSeparator {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 2
        }

        FormCard.FormTextFieldDelegate {
            id: linkUrlField

            label: i18nc("@label:textbox", "Link URL:")
            leftPadding: Kirigami.Units.largeSpacing * 2
            rightPadding: Kirigami.Units.largeSpacing * 2
        }
    }

    footer: QQC2.DialogButtonBox {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2

        standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    }
}
