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

    blockComponent: Text {
        text: blockData.html
        textFormat: Text.RichText
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
    }
}