import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: false

    blockComponent: RowLayout {
        FontMetrics {
            id: fm
            font: Kirigami.Theme.defaultFont
        }

        function getTopMargin(loadedItem) {
            return loadedItem ? Math.max(0, (fm.height - loadedItem.height) / 2) + Kirigami.Units.mediumSpacing : 0;
        }

        Loader {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: getTopMargin(item) - Kirigami.Units.smallSpacing
            active: blockData.listType === MDOptions.ListType.TaskList
            visible: active

            sourceComponent: Controls.CheckBox {
                checked: blockData.isChecked
            }
        }

        Loader {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: getTopMargin(item)
            active: blockData.listType === MDOptions.ListType.UnorderedList
            visible: active

            sourceComponent: Rectangle {
                color: Kirigami.Theme.highlightColor
                implicitWidth: Kirigami.Units.gridUnit / 4.0
                implicitHeight: Kirigami.Units.gridUnit / 4.0
            }
        }

        Loader {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: getTopMargin(item)
            active: blockData.listType === MDOptions.ListType.OrderedList
            visible: active

            sourceComponent: Text {
                text: blockData.listIndex + "."
                font.bold: true
                color: Kirigami.Theme.textColor
            }
        }
    }
}