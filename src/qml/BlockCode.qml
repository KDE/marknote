import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.smallSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    blockComponent: Item {
        implicitWidth: parent.width
        implicitHeight: scrollView.implicitHeight
        
        ScrollView {
            id: scrollView

            implicitWidth: parent.width
            implicitHeight: codeText.implicitHeight + (ScrollBar.horizontal.visible ? ScrollBar.horizontal.height : 0)

            ScrollBar.vertical.policy: ScrollBar.AlwaysOff

            TextArea {
                id: codeText
                text: blockData.text
                font.family: Kirigami.Theme.fixedWidthFont.family
                color: Kirigami.Theme.backgroundColor
                anchors.leftMargin: Kirigami.Units.smallSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing
                padding: Kirigami.Units.largeSpacing

                background: Rectangle {
                    color: Kirigami.Theme.textColor
                    radius: Kirigami.Units.smallSpacing
                }
            }
        }

        RowLayout {
            anchors {
                right: parent.right
                top: parent.top
                rightMargin: Kirigami.Units.mediumSpacing
                topMargin: Kirigami.Units.mediumSpacing
            }

            Button {
                icon.source: "edit-copy-symbolic"
                onClicked: {
                    Qt.callLater(function() {
                        Qt.application.clipboard.setText(blockData.text);
                    });
                }
            }
        }
    }
}