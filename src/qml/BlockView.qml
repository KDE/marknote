import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models

import org.kde.kirigami as Kirigami

Item {
    id: root
    anchors.fill: parent

    required property var richDocumentHandler;

    DelegateModel {
        id: treeDelegateModel
        model: richDocumentHandler.treeModel

        delegate: BlockChooser { }
    }

    RowLayout {
        anchors.fill: parent

        ListView {
            model: treeDelegateModel
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: verticalScrollBar
            synchronousDrag: true

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
        }

        ScrollBar {
            id: verticalScrollBar
            policy: ScrollBar.AlwaysOn
            Layout.fillHeight: true
        }
    }
}
