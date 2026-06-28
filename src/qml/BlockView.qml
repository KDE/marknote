import QtQuick
import QtQuick.Layouts
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

    ListView {
        model: treeDelegateModel        
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit
    }
}
