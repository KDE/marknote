import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.kde.marknote 1.0

Kirigami.Page {
    id: root
    property NoteBooksModel model
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    background: Rectangle {color: Kirigami.Theme.backgroundColor; opacity: 0.6}

    AddNotebookDialog {
        id: addNotebookDialog
        model: root.model

    }
    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        icon.name: "addressbook-details"
        text: "Start by Creating your first Notebook!"
        helpfulAction: Kirigami.Action {
            icon.name: "list-add"
            text: "Add Notebook"
            onTriggered: addNotebookDialog.open()
        }
    }
}


