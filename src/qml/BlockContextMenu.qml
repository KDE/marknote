// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

QQC2.Menu {
    id: root
    
    // Holds the reference to the block that was right-clicked
    property var currentBlock: null

    QQC2.MenuItem {
        action: EditorActions.copyAction
    }

    QQC2.MenuItem {
        action: EditorActions.deleteBlockAction
    }
}
