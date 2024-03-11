// SPDX-FileCopyrightText: 2023 Mathis Brüchert <mbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import QtQuick.Layouts
import org.kde.marknote

/**
* @brief A bottom drawer component with a drag indicator.
*
* Example:
* @code
* Components.BottomDrawer {
*   id: drawer
*   headerContentItem: Kirigami.Heading {
*       text: "Drawer"
*   }
*
*   drawerContentItem: ColumnLayout {
*       Kirigami.BasicListItem {
*           label: "Action 1"
*           icon: "list-add"
*           onClicked: {
*               doSomething()
*               drawer.close()
*           }
*       }
*       Kirigami.BasicListItem{
*           label: "Action 2"
*           icon: "list-add"
*           onClicked: {
*               doSomething()
*               drawer.close()
*           }
*       }
*   }
* }
* @endcode
*/
QQC2.Drawer {
    id: root

    /**
    * @brief This property holds the content item of the drawer
    */
    property alias drawerContentItem: drawerContent.contentItem

    /**
    * @brief This property holds the content item of the drawer header
    *
    * when no headerContentItem is set, the header will not be displayed
    */
    property alias headerContentItem: headerContent.contentItem



    edge: Qt.BottomEdge
    height:contents.implicitHeight
    width: applicationWindow().width

    // makes sure the drawer is not able to be opened when not trigered
    interactive : false

    background: Kirigami.ShadowedRectangle {
        corners {
            topRightRadius: 10
            topLeftRadius: 10
        }

        shadow {
            size: 20
            color: Qt.rgba(0, 0, 0, 0.5)
        }

        color: Kirigami.Theme.backgroundColor
    }

    onAboutToShow: root.interactive = true
    onClosed: root.interactive = false

    ColumnLayout {
        id: contents

        spacing: 0
        anchors.fill: parent

        Kirigami.ShadowedRectangle {
            id: headerBackground

            visible: headerContentItem
            height: header.implicitHeight

            Kirigami.Theme.colorSet: Kirigami.Theme.Header
            color: Kirigami.Theme.backgroundColor

            Layout.fillWidth: true

            corners {
                topRightRadius: 10
                topLeftRadius: 10
            }

            ColumnLayout{
                id:header

                anchors.fill: parent
                spacing:0
                clip: true

                Handle {
                    // drag indicator displayed when there is a headerContentItem
                    id: handle

                    Layout.bottomMargin: 0
                }

                QQC2.Control {
                    id: headerContent

                    topPadding: 0
                    leftPadding: Kirigami.Units.mediumSpacing + handle.height
                    rightPadding: Kirigami.Units.mediumSpacing + handle.height
                    bottomPadding: Kirigami.Units.mediumSpacing + handle.height

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }
            }
        }

        Handle {
            // drag indecator displayed when there is no headerContentItem
            visible: !headerContentItem
        }

        QQC2.Control {
            id: drawerContent

            topPadding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0

            Layout.fillWidth: true
        }
    }
    component Handle: Rectangle {
        color: Kirigami.Theme.textColor
        radius: height / 2
        opacity: 0.5
        width: Kirigami.Units.gridUnit * 2.5
        height: Kirigami.Units.gridUnit / 4

        Layout.margins: Kirigami.Units.mediumSpacing
        Layout.alignment: Qt.AlignHCenter
    }
}
