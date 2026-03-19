// SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.marknote
import org.kde.syntaxhighlighting

EditPage {
    id: root

    objectName: "RawEditPage"

    document: RawDocumentHandler {
        cursorPosition: root.textArea.cursorPosition
        document: root.textArea.textDocument
        selectionStart: root.textArea.selectionStart
        selectionEnd: root.textArea.selectionEnd
        textArea: root.textArea

        onCopy: root.textArea.copy()

        onCut: root.textArea.cut()
        onError: message => {
            console.error("Error message from document handler", message);
        }
        // textColor: TODO
        onLoaded: text => {
            root.textArea.text = text;
        }
        onMoveCursor: position => {
            root.textArea.cursorPosition = position;
        }
        onRedo: root.textArea.redo()
        onSelectCursor: (start, end) => {
            root.textArea.select(start, end);
        }
        onUndo: root.textArea.undo()
    }

    SyntaxHighlighter {
        textEdit: root.textArea
        definition: "Markdown"
    }

    textFieldContextMenu: TextFieldContextMenu {
        document: root.document
    }
}
