/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.12

Kirigami.ApplicationWindow {
    id: root

    title: i18n("marknote")


    pageStack.initialPage: "qrc:/NotesPage.qml"
    globalDrawer: Kirigami.GlobalDrawer {
            Kirigami.Theme.colorSet: Kirigami.Theme.Header

            title: i18n("dfad")
            modal: !wideScreen
            width: 60
            margins: 0
            padding: 0
            header: Kirigami.AbstractApplicationHeader {
            }
            ColumnLayout {

                Kirigami.NavigationTabButton {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    display: Controls.AbstractButton.IconOnly
                    text: collapsed? "" : i18n("Library")
                    icon.name: "file-library-symbolic"
                    Layout.margins: 0
                }
                Kirigami.NavigationTabButton {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    display: Controls.AbstractButton.IconOnly
                    text: collapsed? "" : i18n("Library")
                    icon.name: "file-library-symbolic"
                    Layout.margins: 0

                }
                Kirigami.NavigationTabButton {
                    Layout.fillWidth: true
                    implicitHeight: 50
                    display: Controls.AbstractButton.IconOnly
                    text: collapsed? "" : i18n("Library")
                    icon.name: "file-library-symbolic"
                    Layout.margins: 0

                }
                Item { Layout.fillHeight: true }
            }
        }





}
