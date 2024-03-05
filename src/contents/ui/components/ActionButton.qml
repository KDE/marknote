// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.marknote

Controls.Button {
    id: root

    height: 55
    width: 55

    background: Kirigami.ShadowedRectangle{
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window

        color: if (parent.down) {
            Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6)
        } else if (parent.hovered) {
            Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8)
        } else {
            Kirigami.Theme.backgroundColor
        }

        radius: 10
        border {
            width: 1
            color: if (parent.down){
                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.4)
            } else if(parent.hovered){
                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6)
            } else{
                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
            }
        }

        shadow {
            size: 10
            xOffset: 2
            yOffset: 2
            color: Qt.rgba(0, 0, 0, 0.2)
        }

        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }

        Behavior on border.color {
            ColorAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    contentItem: Item {
        Kirigami.Icon {
            implicitHeight: Kirigami.Units.gridUnit * 1.2
            source: root.icon.name
            anchors.centerIn: parent
        }
    }
}
