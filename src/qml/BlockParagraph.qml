// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

BlockTemplate {
    id: root

    isFinalBlock: true

    topMargin: Kirigami.Units.mediumSpacing
    bottomMargin: Kirigami.Units.mediumSpacing

    readonly property var blockData: root.block.data

    blockComponent: BlockText {
        html: root.blockData.html
        md: root.blockData.md
        blockType: root.blockData.blockType
        delegateModel: root.delegateModel
        block: root.block
        index: root.index
    }
}