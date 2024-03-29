// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Controls as QQC2
import QtQuick.Layouts

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

    leftPadding: Kirigami.Units.largeSpacing * 2
    rightPadding: Kirigami.Units.largeSpacing * 2
    topPadding: 0
    bottomPadding: 0

    header: Kirigami.Heading {
        text: root.title
        level: 2
        elide: QQC2.Label.ElideRight
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: 0
    }

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormTextFieldDelegate {
            id: linkTextField

            label: i18nc("@label:textbox", "Link Text:")
            leftPadding: 0
            rightPadding: 0
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        FormCard.FormTextFieldDelegate {
            id: linkUrlField

            label: i18nc("@label:textbox", "Link URL:")
            leftPadding: 0
            rightPadding: 0
        }
    }

    footer: QQC2.DialogButtonBox {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2

        standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    }
}
