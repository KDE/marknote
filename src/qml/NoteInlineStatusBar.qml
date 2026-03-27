// SPDX-FileCopyrightText: 2026 Oliver Beard <olib141@outlook.com>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Controls.Control {
    id: root

    property string text: ""
    property Item hoverTarget: parent

    // Expose these so the parent page can use them for Dolphin-style hover text later
    property alias targetHovered: hoverHandler.hovered
    property alias targetPoint: hoverHandler.point

    anchors.bottom: parent.bottom
    anchors.bottomMargin: -background.border.width
    z: 99

    readonly property real effectiveWidth: width - background.border.width
    readonly property real effectiveHeight: height - background.border.width

    padding: Kirigami.Units.smallSpacing + background.border.width

    HoverHandler {
        id: hoverHandler
        parent: root.hoverTarget
        target: root.hoverTarget

        readonly property bool swapSides: {
            if (!hovered) return false;

            const p = point.position;
            const px = Math.round(p.x);
            // Adjust for scrolling if the target is a Flickable/ListView
            const py = Math.round(root.hoverTarget instanceof Flickable ? p.y - root.hoverTarget.contentY : p.y);

            return px <= root.effectiveWidth && py >= (root.hoverTarget.height - root.effectiveHeight);
        }
    }

    states: [
        State {
            name: "left"
            when: !hoverHandler.swapSides
            AnchorChanges {
                target: root
                anchors.left: parent.left
                anchors.right: undefined
            }
            PropertyChanges {
                target: root
                anchors.leftMargin: -root.background.border.width
                anchors.rightMargin: 0
                background.topLeftRadius: 0
                background.topRightRadius: Kirigami.Units.cornerRadius
            }
        },
        State {
            name: "right"
            when: hoverHandler.swapSides
            AnchorChanges {
                target: root
                anchors.left: undefined
                anchors.right: parent.right
            }
            PropertyChanges {
                target: root
                anchors.leftMargin: 0
                anchors.rightMargin: -root.background.border.width
                background.topLeftRadius: Kirigami.Units.cornerRadius
                background.topRightRadius: 0
            }
        }
    ]

    opacity: text.length === 0 ? 0 : 1
    Behavior on opacity {
        NumberAnimation {
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.InOutQuad
        }
    }
    visible: opacity > 0

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Kirigami.ColorUtils.linearInterpolation(
            Kirigami.Theme.backgroundColor,
            Kirigami.Theme.textColor,
            Kirigami.Theme.frameContrast
        )
    }

    contentItem: Controls.Label {
        text: root.text
        maximumLineCount: 1
        elide: Text.ElideMiddle
    }
}
