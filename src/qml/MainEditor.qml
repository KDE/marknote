/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.marknote

import "components"

Kirigami.ApplicationWindow {
    id: root

    property bool columnModeDelayed: false

    minimumWidth: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 20

    pageStack {
        globalToolBar {
            style: Kirigami.Settings.isMobile? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
            showNavigationButtons: Config.fillWindow? Kirigami.ApplicationHeaderStyle.None : Kirigami.ApplicationHeaderStyle.ShowBackButton
        }

        defaultColumnWidth: Config.fillWindow? 0 : 15 * Kirigami.Units.gridUnit
        columnView {
            columnResizeMode: Kirigami.ColumnView.SingleColumn
        }

        initialPage: RichEditPage {
            id: editorPage

            Component.onCompleted: {
                editorPage.noteName = cliNoteName;
                editorPage.noteFullPath = cliNoteFullPath;
                editorPage.singleDocumentMode = true;
                root.title = editorPage.noteName
            }
        }
    }

    Connections {
        target: App

        function onOpenAboutPage(): void {
            const openDialogWindow = pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }

        function onOpenAboutKDEPage(): void {
            const openDialogWindow = pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }
    }
}
