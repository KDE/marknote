// SPDX-FileCopyrightText: 2022 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform as Labs
import org.kde.marknote
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs

Labs.Menu {
    id: root

    title: i18nc("@action:menu", "Help")

    StatefulAppLabs.NativeMenuItem {
        actionName: "open_about_page"
        application: App
    }

    StatefulAppLabs.NativeMenuItem {
        actionName: "open_about_kde_page"
        application: App
    }
}
