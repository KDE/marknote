// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtCore
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs

QQC2.Dialog {
    id: root

    readonly property alias imagePath: imageField.path

    x: Math.round((parent.width - width) / 2)
    y: Math.round(parent.height / 3)

    width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 20)

    title: i18nc("@title:window", "Insert Image")

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

    FileDialog {
        id: fileDialog

        title: i18nc("@title:window", "Select an image")
        currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        fileMode: FileDialog.OpenFile
        nameFilters: [i18n("Image files (*.jpg *.jpeg *.png *.svg *.webp)"), i18n("All files (*)")]
        onAccepted: imageField.path = selectedFile
    }

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormButtonDelegate {
            id: imageField

            property url path

            text: i18nc("@label:textbox", "Image Location:")
            description: path.toString().length > 0 ? path.toString().split('/').slice(-1)[0] : ''
            leftPadding: Kirigami.Units.largeSpacing * 2
            rightPadding: Kirigami.Units.largeSpacing * 2

            onClicked: fileDialog.open()
        }

        Item {
            visible: imageField.path.toString().length > 0

            Layout.fillWidth: true
            Layout.preferredHeight: 200
            Layout.topMargin: Kirigami.Units.largeSpacing

            Image {
                anchors.fill: parent
                source: imageField.path
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2

        standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    }
}
