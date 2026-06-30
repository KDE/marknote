import QtQuick
import QtQuick.Layouts

BlockTemplate {
    id: root

    blockComponent: Text {
        text: blockData.html
        textFormat: Text.RichText
    }
}