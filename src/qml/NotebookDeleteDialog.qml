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
    standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

    Controls.Label {
        Layout.fillWidth: true
        text: i18n("Are you sure you want to delete the Notebook <b> %1 </b>? This will delete the content of <b>%2</b> definitively.", removeDialog.name, removeDialog.model.storagePath + '/' + removeDialog.name)
        wrapMode: Text.WordWrap
    }
}
