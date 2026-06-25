import QtQuick
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    required property var richDocumentHandler;

    property var testModel : MDTreeModel {
        id: treeModel
    }

    Component.onCompleted: {
        richDocumentHandler.setMdTreeModel(treeModel)
    }

    ListView {
        id: testView
        model: 10

        delegate: Rectangle {
            required property int index

            width: parent.width
            height: 50
            color: index % 2 === 0 ? "lightgray" : "white"
        }
    }
}
