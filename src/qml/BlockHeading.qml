import QtQuick
import QtQuick.Layouts

BlockTemplate {
    id: myblock

    blockComponent: Text {
        text: blockData.html
        textFormat: Text.RichText
    }
}