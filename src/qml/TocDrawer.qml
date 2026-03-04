// SPDX-FileCopyrightText: 2026 Shubham Shinde <shindeshubham0520@gmail.com>
// SPDX-License-Identifier:  GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts
import org.kde.ki18n
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.marknote

Kirigami.OverlayDrawer {
    id: root

    required property T.TextArea textArea

    modal: false
    handleVisible: false
    edge: Qt.RightEdge

    width: Kirigami.Units.gridUnit * 15
    
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: KI18n.i18nc("@title:window", "Table of Contents")
                Layout.fillWidth: true
                elide: Text.ElideRight
                type: Kirigami.Heading.Type.Primary
            }

            ToolButton {
                icon.name: "dialog-close"
                text: KI18n.i18nc("@action:button", "Close")
                display: AbstractButton.IconOnly
                onClicked: root.close()

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        ListView {
            id: tocListView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: TocModel {
                id: tocModel
                document: root.textArea.textDocument
            }

            clip: true

            delegate: Delegates.RoundedItemDelegate {
                id: tocDelegate
                width: ListView.view.width

                required property string title
                required property int level
                required property int index
                required property int cursorPosition

                text: title
                leftPadding: (level - 1) * Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                highlighted: ListView.isCurrentItem

                onClicked: {
                    ListView.view.currentIndex = index
                    root.textArea.cursorPosition = cursorPosition
                    root.textArea.forceActiveFocus()
                    if (Kirigami.Settings.isMobile) {
                        root.close()
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                icon.name: "format-list-unordered"
                visible: tocListView.count === 0
                text: KI18n.i18n("No headers found")
            }
        }
    }
}
