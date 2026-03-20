/*
    SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2023 Ivan Tkachenko <me@ratijas.tk>
    SPDX-FileCopyrightText: 2026 Valentyn Bondarenko <bondarenko@vivaldi.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQml.Models
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.sonnet as Sonnet
import org.kde.marknote
import org.kde.ki18n

QQC2.Menu {
    id: root

    property T.TextArea target
    property bool deselectWhenMenuClosed: true
    property int restoredCursorPosition: 0
    property int restoredSelectionStart
    property int restoredSelectionEnd
    property bool persistentSelectionSetting
    property TableActionHelper tableActionHelper: null
    property url currentLink
    property var document

    signal internalLinkClicked(link: url)

    // assuming that Instantiator::active is bound to target.Kirigami.SpellCheck.enabled
    property Instantiator/*<Sonnet.SpellcheckHighlighter>*/ spellcheckHighlighterInstantiator

    // assuming that spellchecker's active state is not writable, use target.Kirigami.SpellCheck.enabled instead.
    readonly property Sonnet.SpellcheckHighlighter spellcheckHighlighter:
        spellcheckHighlighterInstantiator?.object as Sonnet.SpellcheckHighlighter

    property /*list<string>*/var spellcheckSuggestions: []

    Component.onCompleted: persistentSelectionSetting = persistentSelectionSetting // break binding

    property var runOnMenuClose: () => {}

    function storeCursorAndSelection() {
        restoredCursorPosition = target.cursorPosition;
        restoredSelectionStart = target.selectionStart;
        restoredSelectionEnd = target.selectionEnd;
    }

    // target is pressed with mouse
    function targetClick(
        handlerPoint,
        target,
        spellcheckHighlighterInstantiator,
        mousePosition,
    ) {
        if (!(target instanceof TextInput || target instanceof TextEdit)) {
            console.warn("Target not supported by standard context menu:", target);
            return;
        }
        if (handlerPoint.pressedButtons === Qt.RightButton) { // only accept just right click
            if (visible) {
                deselectWhenMenuClosed = false; // don't deselect text if menu closed by right click on textfield
                dismiss();
            } else {
                this.target = target;
                target.persistentSelection = true; // persist selection when menu is opened

                this.spellcheckHighlighterInstantiator = spellcheckHighlighterInstantiator;

                spellcheckSuggestions = (spellcheckHighlighter && mousePosition)
                    ? spellcheckHighlighter.suggestions(mousePosition)
                    : [];

                storeCursorAndSelection();
                popup(target);
                // slightly locate context menu away from mouse so no item is selected when menu is opened
                x += 1
                y += 1
            }
        } else {
            dismiss();
        }
    }

    // context menu keyboard key
    function targetKeyPressed(event, target) {
        if (event.modifiers === Qt.NoModifier && event.key === Qt.Key_Menu) {
            this.target = target;
            target.persistentSelection = true; // persist selection when menu is opened
            storeCursorAndSelection();
            popup(target);
        }
    }

    function __hasSelectedText(): bool {
        return target !== null
            && target.selectedText !== "";
    }

    function __editable(): bool {
        return target !== null
            && !target.readOnly;
    }

    function __hasSpellcheckCapability(): bool {
        return __editable()
            && spellcheckHighlighterInstantiator !== null;
    }

    function __showSpellcheckActions(): bool {
        return __editable()
            && spellcheckHighlighter !== null
            && spellcheckHighlighter.active
            && spellcheckHighlighter.wordIsMisspelled;
    }

    function __isInternalLink(link: url): bool {
        if (!link) {
            return false;
        }
        return link.toString().startsWith("marknote://note/");
    }

    modal: true

    // deal with whether text should be deselected
    onClosed: {
        // reset parent, so OverlayZStacking could refresh z order next time
        // this menu is about to open for the same item that might have been
        // reparented to a different popup.
        parent = null;

        // restore text field's original persistent selection setting
        target.persistentSelection = persistentSelectionSetting
        // deselect text field text if menu is closed not because of a right click on the text field
        if (deselectWhenMenuClosed) {
            target.deselect();
        }
        deselectWhenMenuClosed = true;

        // restore cursor position
        target.forceActiveFocus();
        target.cursorPosition = restoredCursorPosition;
        target.select(restoredSelectionStart, restoredSelectionEnd);

        // run action, and free memory
        try {
            runOnMenuClose = () => {};
        } catch (e) {
            console.error(e);
            console.trace();
        }
        runOnMenuClose = () => {};

        // clean up spellchecker
        spellcheckHighlighterInstantiator = null;
        spellcheckSuggestions = [];
    }

    onOpened: {
        runOnMenuClose = () => {};
    }

    Instantiator {
        active: root.__showSpellcheckActions()

        model: root.spellcheckSuggestions
        delegate: QQC2.MenuItem {
            required property string modelData

            text: modelData

            onClicked: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.replaceWord(modelData);
                };
            }
        }
        onObjectAdded: (index, object) => {
            root.insertItem(0, object);
        }
        onObjectRemoved: (index, object) => {
            root.removeItem(object);
        }
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions() && root.spellcheckSuggestions.length === 0
        action: QQC2.Action {
            enabled: false
            text: root.spellcheckHighlighter
                ? KI18n.i18nc("@action:inmenu", 'No Suggestions for "%1"')
                    .arg(root.spellcheckHighlighter.wordUnderMouse)
                : ""
        }
    }

    QQC2.MenuSeparator {
        visible: root.__showSpellcheckActions()
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: QQC2.Action {
            text: root.spellcheckHighlighter
                ? KI18n.i18nc("@action:inmenu", 'Add "%1" to Dictionary')
                    .arg(root.spellcheckHighlighter.wordUnderMouse)
                : ""

            onTriggered: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.addWordToDictionary(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: QQC2.Action {
            text: KI18n.i18nc("@action:inmenu", "Ignore")
            onTriggered: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.ignoreWord(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }

    QQC2.MenuItem {
        visible: root.__hasSpellcheckCapability()

        checkable: true
        checked: root.target?.Kirigami.SpellCheck.enabled ?? false
        text: KI18n.i18nc("@action:inmenu", "Spell Check")

        onToggled: {
            if (root.target) {
                root.target.Kirigami.SpellCheck.enabled = checked;
            }
        }
    }

    QQC2.MenuSeparator {
        visible: root.__hasSpellcheckCapability() && root.__editable()
    }

    QQC2.MenuItem {
        visible: root.currentLink.toString() !== ""
        action: QQC2.Action {
            text: root.__isInternalLink(root.currentLink) ? KI18n.i18nc("@inmenu", "Open Note") : KI18n.i18nc("@inmenu", "Open Link")
            icon.name: "document-open"
            onTriggered: {
                if (root.__isInternalLink(root.currentLink)) {
                    root.internalLinkClicked(root.currentLink);
                } else {
                    Qt.openUrlExternally(root.currentLink);
                }
            }
        }
    }
    QQC2.MenuSeparator {
        visible: root.currentLink.toString() !== ""
    }

    Instantiator {
        id: insertInstantiator
        active: root.tableActionHelper?.actionInsertRowAbove?.enabled ?? false

        delegate: QQC2.Menu {
            title: KI18n.i18nc("@inmenu", "Insert")

            QQC2.MenuItem {
                text: root.tableActionHelper.actionInsertRowAbove.text
                icon.name: "edit-table-insert-row-above"
                onTriggered: root.tableActionHelper.actionInsertRowAbove.trigger()
            }
            QQC2.MenuItem {
                text: root.tableActionHelper.actionInsertRowBelow.text
                icon.name: "edit-table-insert-row-below"
                onTriggered: root.tableActionHelper.actionInsertRowBelow.trigger()
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: root.tableActionHelper.actionInsertColumnBefore.text
                icon.name: "edit-table-insert-column-left"
                onTriggered: root.tableActionHelper.actionInsertColumnBefore.trigger()
            }
            QQC2.MenuItem {
                text: root.tableActionHelper.actionInsertColumnAfter.text
                icon.name: "edit-table-insert-column-right"
                onTriggered: root.tableActionHelper.actionInsertColumnAfter.trigger()
            }
        }

        onObjectAdded: (index, object) => root.insertMenu(root.count - 10, object)
        onObjectRemoved: (index, object) => root.removeMenu(object)
    }

    Instantiator {
        id: removeInstantiator
        active: root.tableActionHelper?.actionRemoveRow?.enabled ?? false

        delegate: QQC2.Menu {
            title: KI18n.i18nc("@inmenu", "Remove")

            QQC2.MenuItem {
                text: root.tableActionHelper.actionRemoveRow.text
                icon.name: "edit-table-delete-row"
                onTriggered: root.tableActionHelper.actionRemoveRow.trigger()
            }
            QQC2.MenuItem {
                text: root.tableActionHelper.actionRemoveColumn.text
                icon.name: "edit-table-delete-column"
                onTriggered: root.tableActionHelper.actionRemoveColumn.trigger()
            }
            QQC2.MenuItem {
                text: root.tableActionHelper.actionRemoveCellContents.text
                icon.name: "deletecell-symbolic"
                onTriggered: root.tableActionHelper.actionRemoveCellContents.trigger()
            }
        }

        onObjectAdded: (index, object) => root.insertMenu(root.count - 10, object)
        onObjectRemoved: (index, object) => root.removeMenu(object)
    }

    QQC2.MenuSeparator {
        visible: insertInstantiator.active || removeInstantiator.active
    }

    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-undo-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Undo")
            shortcut: StandardKey.Undo
        }
        enabled: root.target?.canUndo ?? false
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.undo();
            };
        }
    }
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-redo-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Redo")
            shortcut: StandardKey.Redo
        }
        enabled: root.target?.canRedo ?? false
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.redo();
            };
        }
    }
    QQC2.MenuSeparator {}
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-cut-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Cut")
            shortcut: StandardKey.Cut
        }

        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.cut();
            };
        }
    }
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-copy-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Copy")
            shortcut: StandardKey.Copy
        }
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.copy();
            };
        }
    }
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-paste-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Paste")
            shortcut: StandardKey.Paste
        }
        visible: root.__editable()
        enabled: canPaste ?? false

        property bool canPaste: {
            if (root.document) {
                return root.document.canPaste;
            } else {
                return root.target?.canPaste;
            }
        }

        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                if (root.document && root.document.pasteFromClipboard) {
                    root.document.pasteFromClipboard();
                } else {
                    root.target.paste();
                }
            };
        }
    }
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-delete-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Delete")
            shortcut: StandardKey.Delete
        }
        visible: root.__editable()
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.remove(root.target.selectionStart, root.target.selectionEnd);
            };
        }
    }
    QQC2.MenuSeparator {
        visible: root.target !== null && root.__editable()
    }
    QQC2.MenuItem {
        action: QQC2.Action {
            icon.name: "edit-select-all-symbolic"
            text: KI18n.i18nc("@action:inmenu", "Select All")
            shortcut: StandardKey.SelectAll
        }
        visible: root.target !== null
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.selectAll();
            };
        }
    }
}
