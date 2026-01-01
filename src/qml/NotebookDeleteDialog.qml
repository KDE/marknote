// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Templates as T
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
    width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 26)
    bottomPadding: Kirigami.Units.gridUnit
    title: i18nc("@title:window", "Delete Notebook")
    onRejected: close()
    onAccepted: deleteAction.trigger()
    standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

    footer: GridLayout {
        id: customGridLayoutFooter
        columns: removeDialog._mobileLayout ? 1 : 1 + buttonRepeater.count + 1
        rowSpacing: Kirigami.Units.mediumSpacing
        columnSpacing: Kirigami.Units.mediumSpacing

        FormCard.FormCheckDelegate {
            id: checkbox
            visible: false // needs a config property to save a state, so disable it for now
            text: i18ndc("kirigami-addons6", "@label:checkbox", "Do not show again")
            background: null
            Layout.alignment: Qt.AlignVCenter
        }
        Item {
            visible: !checkbox.visible
            Layout.fillWidth: true
        }

        Repeater {
            id: buttonRepeater
            model: dialogButtonBox.contentModel
        }

        T.DialogButtonBox {
            id: dialogButtonBox
            standardButtons: removeDialog.standardButtons

            // this aligns buttons to the left of the dialog box. Unlike "Layout.alignment: Qt.AlignRight", it just works
            Layout.leftMargin: -(Kirigami.Units.gridUnit * 5)

            contentItem: Item {}

            onAccepted: removeDialog.accepted()
            onDiscarded: removeDialog.discarded()
            onApplied: removeDialog.applied()
            onHelpRequested: removeDialog.helpRequested()
            onRejected: removeDialog.rejected()

            delegate: Controls.Button {
                property int index: buttonRepeater.model.children.indexOf(this)
                Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.DialogButton
                Layout.fillWidth: removeDialog._mobileLayout
                Layout.leftMargin: removeDialog._mobileLayout ? Kirigami.Units.mediumSpacing * 2 : (index === 0 ? Kirigami.Units.mediumSpacing : 0)
                Layout.rightMargin: removeDialog._mobileLayout ? Kirigami.Units.mediumSpacing * 2 : (index === buttonRepeater.count - 1 ? Kirigami.Units.mediumSpacing : 0)
                Layout.bottomMargin: removeDialog._mobileLayout && index !== buttonRepeater.count - 1 ? 0 : Kirigami.Units.mediumSpacing * 2
            }
        }
    }

    Controls.Label {
        Layout.fillWidth: true
        text: i18n("Are you sure you want to delete the Notebook <b>%1</b>? This will delete the content of <b>%2</b> definitively.", removeDialog.name, removeDialog.model.storagePath + '/' + removeDialog.name)
        wrapMode: Text.WordWrap
    }
}
