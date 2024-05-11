// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.marknote

FormCard.FormCardDialog {
    id: root

    title: i18nc("@title:dialog", "Editor Settings")

    standardButtons: Dialog.Ok | Dialog.Reset

    onAccepted: root.close();
    onReset: Config.reset();

    FormCard.FormButtonDelegate {
        text: i18nc("@label:textbox", "Notes Directory:")
        enabled: !Config.isStorageImmutable
        description: Config.storage
        onClicked: folderDialog.open();

        FolderDialog {
            id: folderDialog

            currentFolder: 'file://' + Config.storage
            title: i18nc("@title:window", "Select the notes directory")
            onAccepted: {
                Config.storage = selectedFolder.toString().replace('file://', '');
                console.log(Config.storage);
                Config.save();
            }
        }
    }

    FormCard.FormDelegateSeparator {}

    FormCard.AbstractFormDelegate {
        background: null
        contentItem: ColumnLayout {
            Label {
                text: i18nc("@label:spinbox", "Font family:")
                Layout.fillWidth: true
            }

            ComboBox {
                property bool isInitialising: true

                model: Config.fontFamilies
                enabled: !Config.isEditorFontImmutable
                onCurrentIndexChanged: {
                    if (isInitialising || !enabled) {
                        return;
                    }
                    Config.editorFont.familiy = currentValue;
                    Config.save();
                }

                Component.onCompleted: {
                    currentIndex = indexOfValue(Config.editorFont.family);
                    isInitialising = false;
                }
                Layout.fillWidth: true
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