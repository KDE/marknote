import QtQuick 2.1
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

import org.kde.marknote 1.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

Kirigami.Dialog{
    id: root
    title: "New Notebook"
    padding: Kirigami.Units.largeSpacing
    contentItem: ColumnLayout {
        KQuickAddons.IconDialog {
            id: iconDialog
            onIconNameChanged: iconButton.icon.name = iconName
        }
        Button {
            implicitHeight: Kirigami.Units.gridUnit *4
            implicitWidth: Kirigami.Units.gridUnit *4
            id: iconButton
            Layout.alignment: Qt.AlignHCenter
            onClicked: iconDialog.open()
        }
        RowLayout {
            Label { text: "Name:"}
            TextField{
                id: fileNameInput
            }
            Button { icon.name: "color-management"}
        }

    }
    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Add")
            iconName: "list-add"

        }
    ]
}
