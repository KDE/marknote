// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls

import org.kde.marknote
import org.kde.kirigamiaddons.components as Components
import org.kde.ki18n

Components.MessageDialog {
    id: removeDialog

    property string path
    property string name
    property NoteBooksModel model
    NotebookDeleteAction {
        id: deleteAction
        name: name
        path: path
        model: model
    }
    dialogType: Components.MessageDialog.Warning
    title: KI18n.i18nc("@title:window", "Delete Notebook")
    onRejected: removeDialog.close()
    onAccepted: deleteAction.trigger()
    standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

    subtitle: KI18n.i18n("Are you sure you want to delete the Notebook <b>%1</b>? This will delete the content of <b>%2</b> definitively.", removeDialog.name, removeDialog.model.storagePath + '/' + removeDialog.name)
}
