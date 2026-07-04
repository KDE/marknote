import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true

    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.mediumSpacing

    blockComponent: Text {
        text: blockData.html
        textFormat: Text.RichText
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
    }
}