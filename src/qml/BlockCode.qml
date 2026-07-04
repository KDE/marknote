import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.smallSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: scrollView.implicitHeight
        
        Flickable {
            id: scrollView

            implicitWidth: parent.width
            implicitHeight: codeText.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)

            contentWidth: codeText.width
            contentHeight: codeText.implicitHeight
            
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            
            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            TextArea {
                id: codeText
                text: blockData.text
                font.family: Kirigami.Theme.fixedWidthFont.family
                color: Kirigami.Theme.backgroundColor
                padding: Kirigami.Units.largeSpacing

                width: Math.max(implicitWidth, scrollView.width)
                implicitHeight: Math.max(buttonRow.implicitHeight + buttonRow.anchors.topMargin * 2, contentHeight + topPadding + bottomPadding)

                background: Rectangle {
                    color: Kirigami.Theme.textColor
                    radius: Kirigami.Units.smallSpacing
                }
            }
        }

        RowLayout {
            id: buttonRow

            anchors {
                right: parent.right
                top: parent.top
                rightMargin: Kirigami.Units.mediumSpacing
                topMargin: Kirigami.Units.mediumSpacing
            }

            KQuickControlsAddons.Clipboard { id: clipboard }

            Button {
                icon.source: "edit-copy-symbolic"
                onClicked: {
                    clipboard.content = codeText.text
                    showPassiveNotification(i18n("Copied to clipboard!"))
                }
            }
        }
    }
}