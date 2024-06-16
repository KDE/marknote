// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 Gary Wang <opensource@blumia.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.marknote

FormCard.FormCardPage {
    title: i18nc("@title:window", "General")

    FormCard.FormHeader {
        title: i18n("Editor Settings")
    }

    FormCard.FormCard {
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
                QQC2.Label {
                    text: i18nc("@label:spinbox", "Font family:")
                    Layout.fillWidth: true
                }

                QQC2.ComboBox {
                    id: fontFamilyComboBox

                    property bool isInitialising: true
                    Layout.fillWidth: true
                    model: Config.fontFamilies
                    enabled: !Config.isEditorFontImmutable
                    onCurrentIndexChanged: {
                        if (isInitialising && !enabled) {
                            return;
                        }
                        Config.editorFont.famliy = currentValue;
                        Config.save();
                    }

                    Component.onCompleted: {
                        currentIndex = indexOfValue(Config.editorFont.family)
                        isInitialising = false
                    }

                    Connections {
                        target: Config
                        function onEditorFontChanged() {
                            fontFamilyComboBox.currentIndex = fontFamilyComboBox.indexOfValue(Config.editorFont.family)
                        }
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormSpinBoxDelegate {
            id: fontSizeSpinbox
            from: 0
            to: 25
            value: Config.editorFont.pixelSize
            label: i18nc("@label:spinbox", "Font size:")
            enabled: !Config.isEditorFontImmutable
            onValueChanged: {
                Config.editorFont.pixelSize = value;
                Config.save();
            }

            Connections {
                target: Config
                function onEditorFontChanged() {
                    fontSizeSpinbox.value = Config.editorFont.pixelSize;
                }
            }
        }
    }

    footer: QQC2.ToolBar {
        height: visible ? implicitHeight : 0
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                text: i18n("Reset")
                onClicked: {
                    Config.reset()
                }
            }
        }
    }
}
