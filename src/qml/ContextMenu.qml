// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import "components"

QtObject {
    id: root

    default property list<T.Action> actions
    property Item visualParent

    function open(): void {
        if (Kirigami.Settings.isMobile) {
            drawer.open();
        } else {
            menu.popup();
        }
    }

    property BottomDrawer drawer: BottomDrawer {
        parent: root.visualParent

        drawerContentItem: ColumnLayout{
            id: contents
            spacing: 0

            Repeater {
                model: root.actions

                Delegates.RoundedItemDelegate {
                    required property T.Action modelData
                    required property int index

                    action: modelData

                    onClicked: drawer.close();

                    Layout.fillWidth: true
                }
            }

            Item { height: Kirigami.Units.largeSpacing * 3}
        }
    }

    property Menu menu: Menu {
        parent: root.visualParent

        Repeater {
            model: root.actions

            MenuItem {
                required property T.Action modelData

                action: modelData

                onClicked: menu.dismiss()
            }
        }
    }
}
