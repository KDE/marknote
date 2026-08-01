// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commandmanager.h"
#include "deindentlistitem.h"
#include "edittext.h"
#include "indentlistitem.h"
#include "mergewithpreviousblock.h"
#include "moveoutsideblockquote.h"
#include "parseblock.h"
#include "removeblockquote.h"
#include "removefromlist.h"
#include "splitblock.h"
#include "splitlistitem.h"
#include "transformtoblockquote.h"
#include "transformtochecklist.h"
#include "transformtolist.h"
#include <QRegularExpression>

using namespace Qt::StringLiterals;

CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
{
}

CommandManager::~CommandManager()
{
}

void CommandManager::setModel(MDTreeModel *model)
{
    m_model = model;
}

void CommandManager::undo()
{
    m_undoStack.undo();
}

void CommandManager::redo()
{
    m_undoStack.redo();
}

bool CommandManager::canUndo() const
{
    return m_undoStack.canUndo();
}

bool CommandManager::canRedo() const
{
    return m_undoStack.canRedo();
}

void CommandManager::splitBlock(TreeItem *block, const QString &text, int splitIndex)
{
    m_undoStack.push(new SplitBlockCommand(block, text, splitIndex, m_model));
}

void CommandManager::editText(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition)
{
    m_undoStack.push(new EditTextCommand(block, oldText, newText, oldCursorPosition, newCursorPosition, m_model));
}

void CommandManager::parseBlock(TreeItem *block, const QString &text)
{
    m_undoStack.push(new ParseBlockCommand(block, text, m_model));
}

void CommandManager::mergeWithPreviousBlock(TreeItem *block, const QString &text)
{
    if (!block || !block->parent()) {
        return;
    }

    TreeItem *target = getPreviousSibling(block);

    if (target) {
        m_undoStack.push(new MergeWithPreviousBlockCommand(block, text, target, m_model));
    }
}

void CommandManager::transformToBlockquote(TreeItem *block, int level, const QString &text)
{
    m_undoStack.push(new TransformToBlockquoteCommand(block, level, text, m_model));
}

void CommandManager::transformToList(TreeItem *block, bool isOrdered, int startNumber, const QString &text)
{
    m_undoStack.push(new TransformToListCommand(block, isOrdered, startNumber, text, m_model));
}

void CommandManager::transformToChecklist(TreeItem *block, bool isChecked, const QString &text)
{
    m_undoStack.push(new TransformToChecklistCommand(block, isChecked, text, m_model));
}

void CommandManager::splitListItem(TreeItem *block, const QString &text, int splitIndex)
{
    if (!block->parent() || block->parent()->type() != MDOptions::ElementType::ListItem) {
        return;
    }

    m_undoStack.push(new SplitListItemCommand(block, text, splitIndex, m_model));
}

bool CommandManager::deIndentListItem(TreeItem *block, int cursorPosition)
{
    const auto listItem = block->parent();
    if (!listItem || listItem->type() != MDOptions::ElementType::ListItem) {
        return false;
    }

    const auto curList = listItem->parent();
    if (!curList || curList->type() != MDOptions::ElementType::List) {
        return false;
    }

    const auto parentListItem = curList->parent();
    if (!parentListItem || parentListItem->type() != MDOptions::ElementType::ListItem) {
        return false;
    }

    const auto parentList = parentListItem->parent();
    if (!parentList || parentList->type() != MDOptions::ElementType::List) {
        return false;
    }

    m_undoStack.push(new DeIndentListItemCommand(block, m_model, cursorPosition));
    return true;
}

void CommandManager::indentListItem(TreeItem *block, int cursorPosition)
{
    const auto listItem = block->parent();
    if (!listItem || listItem->type() != MDOptions::ElementType::ListItem) {
        return;
    }

    const auto curList = listItem->parent();
    if (!curList || curList->type() != MDOptions::ElementType::List) {
        return;
    }

    if (listItem->row() <= 0) {
        return;
    }

    m_undoStack.push(new IndentListItemCommand(block, m_model, cursorPosition));
}

bool CommandManager::convertToParagraph(TreeItem *block, int cursorPosition)
{
    TreeItem *node = block;
    TreeItem *parent = block->parent();

    while (parent && parent->type() != MDOptions::ElementType::Document) {
        if (node->row() != 0) {
            return false;
        }

        if (parent->type() == MDOptions::ElementType::Blockquote) {
            return removeBlockquoteIfAtStart(parent, block, cursorPosition);
        } else if (parent->type() == MDOptions::ElementType::ListItem) {
            return removeFromListIfAtStart(block, cursorPosition);
        }

        node = parent;
        parent = parent->parent();
    }

    return false;
}

bool CommandManager::moveOutsideBlockquote(TreeItem *block, int cursorPosition)
{
    TreeItem *parent = block->parent();
    if (!parent || parent->type() != MDOptions::ElementType::Blockquote) {
        return false;
    }

    if (block->row() != parent->childCount() - 1) {
        return false;
    }

    m_undoStack.push(new MoveOutsideBlockquoteCommand(block, m_model, cursorPosition));
    return true;
}

bool CommandManager::removeBlockquoteIfAtStart(TreeItem *bqBlock, TreeItem *block, int cursorPosition)
{
    m_undoStack.push(new RemoveBlockquoteCommand(bqBlock, block, m_model, cursorPosition));
    return true;
}

bool CommandManager::removeFromListIfAtStart(TreeItem *block, int cursorPosition)
{
    m_undoStack.push(new RemoveFromListCommand(block, m_model, cursorPosition));
    return true;
}

void CommandManager::moveToPreviousBlock(TreeItem *block, int cursorPosition)
{
    TreeItem *target = getPreviousSibling(block);
    if (target) {
        m_model->requestFocus(target, cursorPosition);
    }
}

void CommandManager::moveToNextBlock(TreeItem *block, int cursorPosition)
{
    TreeItem *target = getNextSibling(block);
    if (target) {
        m_model->requestFocus(target, cursorPosition);
    }
}

TreeItem *CommandManager::getPreviousSibling(TreeItem *block)
{
    TreeItem *current = block;

    while (current) {
        if (current->row() > 0) {
            current = current->parent()->child(current->row() - 1);
            while (current->childCount() > 0) {
                current = current->child(current->childCount() - 1);
            }
        } else {
            current = current->parent();
        }

        if (current) {
            const auto data = current->data();
            if (data.contains(u"md"_s) || data.contains(u"text"_s)) {
                return current;
            }
        }

        if (current && !current->parent()) {
            break;
        }
    }

    return nullptr;
}

TreeItem *CommandManager::getNextSibling(TreeItem *block)
{
    TreeItem *current = block;

    while (current) {
        if (current->childCount() > 0) {
            current = current->child(0);
        } else {
            while (current && current->parent() && current->row() == current->parent()->childCount() - 1) {
                current = current->parent();
            }
            if (current && current->parent()) {
                current = current->parent()->child(current->row() + 1);
            } else {
                current = nullptr;
            }
        }

        if (current) {
            const auto data = current->data();
            if (data.contains(u"md"_s) || data.contains(u"text"_s)) {
                return current;
            }
        }
    }

    return nullptr;
}

bool CommandManager::autoTransform(TreeItem *block, const QString &text, int cursorPosition, int index)
{
    QString textBeforeCursor = text.left(cursorPosition);
    QString remainingText = text.mid(cursorPosition);

    // Check blockquote
    QRegularExpression matchBlockquote(u"^>+$"_s);
    QRegularExpressionMatch bqMatch = matchBlockquote.match(textBeforeCursor);
    if (bqMatch.hasMatch()) {
        int count = bqMatch.captured(0).length();
        transformToBlockquote(block, count, remainingText);
        return true;
    }

    bool isFirstChildOfListItem = (block->parent() && block->parent()->type() == MDOptions::ElementType::ListItem && index == 0);

    if (isFirstChildOfListItem) {
        // Check checklist
        QRegularExpression matchChecklist(u"^\\[([ x])\\]$"_s);
        QRegularExpressionMatch clMatch = matchChecklist.match(textBeforeCursor);
        if (clMatch.hasMatch()) {
            bool isChecked = (clMatch.captured(1) == u"x"_s);
            transformToChecklist(block, isChecked, remainingText);
            return true;
        }
    } else {
        // Check unordered list
        QRegularExpression matchUnordered(u"^[-+*]$"_s);
        if (matchUnordered.match(textBeforeCursor).hasMatch()) {
            transformToList(block, false, 0, remainingText);
            return true;
        }

        // Check ordered list
        QRegularExpression matchOrdered(u"^(\\d+)[.)]$"_s);
        QRegularExpressionMatch olMatch = matchOrdered.match(textBeforeCursor);
        if (olMatch.hasMatch()) {
            int startNumber = olMatch.captured(1).toInt();
            transformToList(block, true, startNumber, remainingText);
            return true;
        }
    }

    return false;
}