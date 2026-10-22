// SPDX-FileCopyrightText: 2026 Mathis Brüchert <mbb@kaidan.im>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQml.Models

import org.kde.marknote
import org.kde.ki18n

import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami

FormCard.FormCardDialog {
    modal: false
    header: Item {}
    width: Kirigami.Units.gridUnit * 18
    onClosed: Config.save()
    FormCard.FormHeader {
        title: KI18n.i18n("Font Family")
    }

    FormCard.FormRadioSelectorDelegate {
        consistentWidth: true
        actions: [
            Kirigami.Action {
                text: KI18n.i18n("Sans")
                icon.name: "font-symbolic"
                onTriggered: Config.editorFont.family = "Sans"
            },
            Kirigami.Action {
                text: KI18n.i18n("Serif")
                icon.name: "format-text-bold"
                onTriggered: Config.editorFont.family = "Serif"
            }
        ]
        selectedIndex: Config.editorFont.family === "Serif" ? 1 : 0
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormHeader {
        title: KI18n.i18n("Font Size")
    }

    FormCard.FormRadioSelectorDelegate {
        consistentWidth: true
        actions: [
            Kirigami.Action {
                text: KI18n.i18n("Small")
                icon.name: "format-font-size-less"
                onTriggered: Config.editorFont.pixelSize = "11"
            },
            Kirigami.Action {
                text: KI18n.i18n("Medium")
                icon.name: "gnumeric-font"
                onTriggered: Config.editorFont.pixelSize = "15"
            },
            Kirigami.Action {
                text: KI18n.i18n("Large")
                icon.name: "format-font-size-more"
                onTriggered: Config.editorFont.pixelSize = "19"
            }
        ]
        selectedIndex: if (Config.editorFont.pixelSize < 15) {
                            return 0
                       } else if (Config.editorFont.pixelSize < 19) {
                            return 1
                       } else {
                            return 2
                       }

    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormHeader {
        title: KI18n.i18n("Text Width")
    }

    FormCard.FormRadioSelectorDelegate {
        consistentWidth: true
        actions: [
            Kirigami.Action {
                text: KI18n.i18n("Restricted")
                icon.name: "format-justify-center"
                onTriggered: Config.fullWidth = false
            },
            Kirigami.Action {
                text: KI18n.i18n("Full")
                icon.name: "format-justify-fill"
                onTriggered: Config.fullWidth = true
            }
        ]
        selectedIndex: Config.fullWidth === true ? 1 : 0
    }
}
