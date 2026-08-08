// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commandmanager.h"
#include "deindentlistitem.h"
#include "editcelltext.h"
#include "editcode.h"
#include "edittext.h"
#include "indentlistitem.h"
#include "insertcolumnintablecommand.h"
#include "insertparagraphbelow.h"
#include "insertrowintablecommand.h"
#include "mergewithpreviousblock.h"
#include "moveoutsideblockquote.h"
#include "parseblock.h"
#include "removeblockquote.h"
#include "removecolumnfromtablecommand.h"
#include "removefromlist.h"
#include "removerowfromtablecommand.h"
#include "splitblock.h"
#include "splitcode.h"
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

void CommandManager::splitCode(TreeItem *block, const QString &oldText, const QString &text, int cursorPosition)
{
    m_undoStack.push(new SplitCodeCommand(block, oldText, text, cursorPosition, m_model));
}

void CommandManager::insertParagraphBelow(TreeItem *block, const QString &text)
{
    m_undoStack.push(new InsertParagraphBelowCommand(block, text, m_model));
}

void CommandManager::editText(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition)
{
    m_undoStack.push(new EditTextCommand(block, oldText, newText, oldCursorPosition, newCursorPosition, m_model));
}

void CommandManager::editCode(TreeItem *block, const QString &oldText, const QString &newText, int oldCursorPosition, int newCursorPosition)
{
    m_undoStack.push(new EditCodeCommand(block, oldText, newText, oldCursorPosition, newCursorPosition, m_model));
}

void CommandManager::editTableCellText(TreeItem *block,
                                       int row,
                                       int column,
                                       const QString &oldText,
                                       const QString &newText,
                                       int oldCursorPosition,
                                       int newCursorPosition)
{
    m_undoStack.push(new EditCellTextCommand(block, row, column, oldText, newText, oldCursorPosition, newCursorPosition, m_model));
}

void CommandManager::parseBlock(TreeItem *block, const QString &text)
{
    m_undoStack.push(new ParseBlockCommand(block, text, m_model));
}

void CommandManager::insertRowInTable(TreeItem *block)
{
    m_undoStack.push(new InsertRowInTableCommand(block, m_model));
}

void CommandManager::insertColInTable(TreeItem *block)
{
    m_undoStack.push(new InsertColumnInTableCommand(block, m_model));
}

void CommandManager::deleteRowInTable(TreeItem *block, int row)
{
    m_undoStack.push(new RemoveRowFromTableCommand(block, row, m_model));
}

void CommandManager::deleteColumnInTable(TreeItem *block, int col)
{
    m_undoStack.push(new RemoveColumnFromTableCommand(block, col, m_model));
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

QString CommandManager::getBlockText(TreeItem *block) const
{
    if (!block) {
        return QString();
    }

    const auto data = block->data();
    if (data.contains(u"md"_s)) {
        return data[u"md"_s].value<QString>();
    } else if (data.contains(u"text"_s)) {
        return data[u"text"_s].value<QString>();
    }

    return QString();
}

void CommandManager::moveToPreviousBlock(TreeItem *block, const QString &currentText, int cursorPosition)
{
    TreeItem *target = getPreviousSibling(block);
    if (!target) {
        return;
    }

    if (target->type() == MDOptions::ElementType::Table) {
        int rowCount = target->data()[u"rowCount"_s].toInt();
        int columnCount = target->data()[u"columnCount"_s].toInt();
        m_model->requestFocusOnTable(target, rowCount - 1, columnCount - 1, -1);
        return;
    }

    int currentNewline = -1;
    if (cursorPosition > 0) {
        currentNewline = currentText.lastIndexOf(u'\n', cursorPosition - 1);
    }
    int currentColumn = cursorPosition - currentNewline - 1;

    QString targetText = getBlockText(target);
    int lastNewline = targetText.lastIndexOf(u'\n');
    int targetPos = 0;

    if (lastNewline != -1) {
        int lastLineLength = targetText.length() - lastNewline - 1;
        targetPos = lastNewline + 1 + qMin(currentColumn, lastLineLength);
    } else {
        targetPos = qMin(currentColumn, static_cast<int>(targetText.length()));
    }

    m_model->requestFocus(target, targetPos);
}

void CommandManager::moveToNextBlock(TreeItem *block, const QString &currentText, int cursorPosition)
{
    TreeItem *target = getNextSibling(block);
    if (!target) {
        return;
    }

    if (target->type() == MDOptions::ElementType::Table) {
        m_model->requestFocusOnTable(target, 0, 0, 0);
        return;
    }

    int currentNewline = -1;
    if (cursorPosition > 0) {
        currentNewline = currentText.lastIndexOf(u'\n', cursorPosition - 1);
    }
    int currentColumn = cursorPosition - currentNewline - 1;

    QString targetText = getBlockText(target);
    int firstNewline = targetText.indexOf(u'\n');
    int targetPos = 0;

    if (firstNewline != -1) {
        targetPos = qMin(currentColumn, firstNewline);
    } else {
        targetPos = qMin(currentColumn, static_cast<int>(targetText.length()));
    }

    m_model->requestFocus(target, targetPos);
}

void CommandManager::moveToLeftTableCell(TreeItem *block, int row, int column)
{
    if (column > 0) {
        m_model->requestFocusOnTable(block, row, column - 1, -1);
    } else if (row > 0) {
        int columnCount = block->data()[u"columnCount"_s].toInt();
        m_model->requestFocusOnTable(block, row - 1, columnCount - 1, -1);
    } else {
        moveToPreviousBlock(block, u""_s, 0);
    }
}

void CommandManager::moveToRightTableCell(TreeItem *block, int row, int column)
{
    int columnCount = block->data()[u"columnCount"_s].toInt();
    int rowCount = block->data()[u"rowCount"_s].toInt();

    if (column < columnCount - 1) {
        m_model->requestFocusOnTable(block, row, column + 1, 0);
    } else if (row < rowCount - 1) {
        m_model->requestFocusOnTable(block, row + 1, 0, 0);
    } else {
        moveToNextBlock(block, u""_s, 0);
    }
}

void CommandManager::moveToTopTableCell(TreeItem *block, int row, int column, int cursorPosition)
{
    if (row > 0) {
        m_model->requestFocusOnTable(block, row - 1, column, cursorPosition);
    } else {
        moveToPreviousBlock(block, u""_s, cursorPosition);
    }
}

void CommandManager::moveToBottomTableCell(TreeItem *block, int row, int column, int cursorPosition)
{
    int rowCount = block->data()[u"rowCount"_s].toInt();

    if (row < rowCount - 1) {
        m_model->requestFocusOnTable(block, row + 1, column, cursorPosition);
    } else {
        moveToNextBlock(block, u""_s, cursorPosition);
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
            if (data.contains(u"md"_s) || data.contains(u"text"_s) || current->type() == MDOptions::ElementType::Table) {
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
            if (data.contains(u"md"_s) || data.contains(u"text"_s) || current->type() == MDOptions::ElementType::Table) {
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

int CommandManager::getCursorInMdString(const QString &rawString, const QString &mdString, int index)
{
    int mdIndex = 0;
    QString cleanedMdString = mdString;
    cleanedMdString.replace(QRegularExpression(u"(\\r\\n|\\n|\\r)"_s), u" "_s);

    int limit = qMin(index, static_cast<int>(rawString.length()));

    for (int rawIndex = 0; rawIndex < limit; rawIndex++) {
        while (mdIndex < cleanedMdString.length() && rawString.at(rawIndex) != cleanedMdString.at(mdIndex)) {
            mdIndex++;
        }

        mdIndex++;
    }

    return qMin(mdIndex, static_cast<int>(cleanedMdString.length()));
}