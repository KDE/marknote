/*
 *    SPDX-License-Identifier: GPL-2.0-or-later
 *    SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
 */

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.marknote

Kirigami.ApplicationWindow {
    id: root

    property bool columnModeDelayed: false

    minimumWidth: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 20

    color: "transparent"
    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        color: Config.useTranslucentBackground
        ? Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, Config.backgroundOpacity)
        : Kirigami.Theme.backgroundColor
    }

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
            const openDialogWindow = root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }

        function onOpenAboutKDEPage(): void {
            const openDialogWindow = root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage"), {
                width: root.width
            }, {
                width: Kirigami.Units.gridUnit * 30,
                height: Kirigami.Units.gridUnit * 30
            });
        }
    }

    Connections {
        target: NavigationController

        function onSourceModeChanged(): void {
            if (root.pageStack.depth >= 1) {
                let current = root.pageStack.items[0];
                let isRaw = current && current.objectName === "RawEditPage";
                let isRich = current && current.objectName === "RichEditPage";
                if (NavigationController.sourceMode && isRich) {
                    let oldPage = root.pageStack.pop();
                    if (oldPage) oldPage.destroy();
                    let comp = Qt.createComponent("org.kde.marknote", "RawEditPage");
                    let newPage = comp.createObject(root.pageStack, {
                        noteName: cliNoteName,
                        noteFullPath: cliNoteFullPath,
                        singleDocumentMode: true
                    });
                    root.pageStack.push(newPage);
                } else if (!NavigationController.sourceMode && isRaw) {
                    let oldPage = root.pageStack.pop();
                    if (oldPage) oldPage.destroy();
                    let comp = Qt.createComponent("org.kde.marknote", "RichEditPage");
                    let newPage = comp.createObject(root.pageStack, {
                        noteName: cliNoteName,
                        noteFullPath: cliNoteFullPath,
                        singleDocumentMode: true
                    });
                    root.pageStack.push(newPage);
                }
            }
        }
    }
}
