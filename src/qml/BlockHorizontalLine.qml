import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true

    blockComponent: Item {
        height: Kirigami.Units.gridUnit
        width: parent.width

        Rectangle {
            color: Kirigami.Theme.alternateBackgroundColor.darker(1.25)
            height: Kirigami.Units.gridUnit / 10.0
            width: parent.width
            anchors.centerIn: parent
        }
    }
}