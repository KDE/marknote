import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    blockComponent: Item {
        implicitWidth: rectangle.width + Kirigami.Units.largeSpacing

        Rectangle {
            id: rectangle

            color: Kirigami.Theme.highlightColor
            implicitWidth: Kirigami.Units.gridUnit / 4.0
            height: parent.height
        }
    }
}