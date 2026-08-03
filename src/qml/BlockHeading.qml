// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true
    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.largeSpacing

    readonly property var blockData: root.block.data

    blockComponent: BlockText {
        implicitWidth: parent.width
        html: root.blockData.html
        md: root.blockData.md
        blockType: root.blockData.blockType
        block: root.block
        delegateModel: root.delegateModel
        index: root.index
    }
}