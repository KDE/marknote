// SPDX-FileCopyrightText: 2024 Gary Wang <opensource@blumia.net>
// SPDX-License-Identifier: LGPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings
import QtQuick.Layouts

KirigamiSettings.ConfigurationsView {
    id: root

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18nc("@action:button", "General")
            icon.name: "org.kde.marknote"
            page: () => Qt.createComponent("org.kde.marknote.settings", "MarkNoteGeneralPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutMarkNote"
            text: i18nc("@action:button", "About MarkNote")
            icon.name: "help-about"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutKDE"
            text: i18nc("@action:button", "About KDE")
            icon.name: "kde"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDE")
        }
    ]
}
