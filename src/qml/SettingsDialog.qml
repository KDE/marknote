// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.marknote

FormCard.FormCardDialog {
    id: root

    title: i18nc("@title:dialog", "Editor Settings")

    standardButtons: Dialog.Ok | Dialog.Reset

    onAccepted: root.close();
    onReset: Config.reset();

    FormCard.AbstractFormDelegate {
        background: null
        contentItem: ColumnLayout {
            Label {
                text: i18nc("@label:spinbox", "Font family:")
                Layout.fillWidth: true
            }

            ComboBox {
                Layout.fillWidth: true
                model: Config.fontFamilies
                enabled: !Config.isEditorFontImmutable
                onCurrentIndexChanged: {
                    Config.editorFont.familiy = currentValue;
                    Config.save();
                }

                Component.onCompleted: currentIndex = indexOfValue(Config.editorFont.family)
            }
        }
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormSpinBoxDelegate {
        from: 0
        to: 25
        value: Config.editorFont.pixelSize
        label: i18nc("@label:spinbox", "Font size:")
        enabled: !Config.isEditorFontImmutable
        onValueChanged: {
            Config.editorFont.pixelSize = value;
            Config.save();
        }
    }
}