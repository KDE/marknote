import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models

import org.kde.kirigami as Kirigami

Item {
    id: root

    required property var index;
    required property var blockData;

    width: ListView.view ? ListView.view.width : 0
    implicitHeight: row.implicitHeight

    property var parentModel: ListView.view ? ListView.view.model : null
    property var cppModel: parentModel ? parentModel.model : null
    property var nodeIndex: parentModel ? parentModel.modelIndex(index) : null

    required property Component blockComponent;

    RowLayout {
        id: row

        Loader {
            id: blockLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: root.blockComponent
        }

        ListView {
            id: childWrapper

            Layout.fillWidth: true
            implicitHeight: contentHeight

            spacing: Kirigami.Units.mediumSpacing

            model: DelegateModel {
                id: childDelegateModel
                model: root.cppModel
                rootIndex: root.nodeIndex

                delegate: root.parentModel ? root.parentModel.delegate : null 
            }
        }
    }
}