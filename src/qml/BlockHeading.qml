import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    blockComponent: Text {
        text: blockData.html
        textFormat: Text.RichText
        color: Kirigami.Theme.textColor
        wrapMode: Text.Wrap
    }
}