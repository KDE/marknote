import QtQuick
import QtQuick.Layouts

BlockTemplate {
    id: myblock

    blockComponent: Rectangle {
        color: "blue"
        implicitWidth: 5
        implicitHeight: 50
    }
}