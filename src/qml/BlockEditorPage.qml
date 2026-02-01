// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQml.Models
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.marknote

Kirigami.ScrollablePage {
    id: root

    objectName: "EditPage"

    ListView {
        model: TextBlockModel {
            id: blockModel
        }

        spacing: Kirigami.Units.smallSpacing

        delegate: DelegateChooser {
            role: "blockType"

            DelegateChoice {
                roleValue: TextBlockModel.Paragraph
                delegate: paragraphBlock
            } 

            DelegateChoice {
                roleValue: TextBlockModel.Heading
                delegate: headingBlock
            } 

            DelegateChoice {
                roleValue: TextBlockModel.Todo
                delegate: todoBlock
            } 
        }
    }

    Component {
        id: paragraphBlock

        TextEdit {
            required property int index
            required property int blockType
            required property var blockData

            width: ListView.view.width
            text: blockData.text
            wrapMode: Text.Wrap
            onTextEdited: blockModel.setData(blockModel.index(index, 0), {text: text}, TextBlockModel.DataRole)
        }
    }

    Component {
        id: headingBlock

        TextField {
            required property int index
            required property var blockData

            width: ListView.view.width
            font.pixelSize: 24
            font.bold: true
            text: blockData.text
            onTextEdited: blockModel.setData(blockModel.index(index, 0), {text: text}, TextBlockModel.DataRole)
        }
    }

    Component {
        id: todoBlock

        Row {
            id: todoBlockDelegate

            required property int index
            required property var blockData

            width: ListView.view.width
            spacing: Kirigami.Units.smallSpacing

            CheckBox {
                checked: todoBlockDelegate.blockData.done
                onToggled: blockModel.setData(blockModel.index(todoBlockDelegate.index, 0), {
                    text: todoBlockDelegate.blockData.text,
                    done: checked,
                }, TextBlockModel.DataRole)
            }

            TextField {
                text: todoBlockDelegate.blockData.text
                onTextEdited: blockModel.setData(blockModel.index(todoBlockDelegate.index, 0), {
                    text: text,
                    done: todoBlockDelegate.blockData.done,
                }, TextBlockModel.DataRole)
            }
        }
    }
}
