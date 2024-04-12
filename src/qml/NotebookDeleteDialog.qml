// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Qt.labs.platform

import org.kde.marknote
import org.kde.iconthemes as KIconThemes
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami

Components.MessageDialog {
    id: removeDialog

    property string path
    property string name
    property NoteBooksModel model
    NotebookDeleteAction {
        id: deleteAction
        name: root.name
        path: root.path
        model: root.model
    }
    dialogType: Components.MessageDialog.Warning
    width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 20)
    height: implicitHeight
    title: i18nc("@title:window", "Delete Notebook")
    onRejected: close()
    onAccepted: deleteAction.trigger()
    standardButtons: Controls.Dialog.Yes | Controls.Dialog.Cancel

    contentItem: Controls.Label {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.largeSpacing
        text: i18n("Are you sure you want to delete the Notebook <b> %1 </b>?", removeDialog.name)
        wrapMode: Text.WordWrap
    }

    footer: Controls.DialogButtonBox {
        leftPadding: Kirigami.Units.largeSpacing * 2
        rightPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.largeSpacing * 2

        standardButtons: removeDialog.standardButtons
    }
}
