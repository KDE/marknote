/*
  SPDX-FileCopyrightText: 2012-2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "tableactionhelper.h"

using namespace Qt::StringLiterals;

#include <KLocalizedString>
#include <QIcon>

#include <QPointer>
#include <QTextTable>

void TableActionHelper::_k_slotRemoveCellContents()
{
    QTextTable *table = textCursor().currentTable();
    const QTextTableCell cell = table->cellAt(textCursor());
    if (cell.isValid()) {
        const QTextCursor firstCursor = cell.firstCursorPosition();
        const QTextCursor endCursor = cell.lastCursorPosition();
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.setPosition(firstCursor.position());
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endCursor.position() - firstCursor.position());
        cursor.removeSelectedText();
        cursor.endEditBlock();
    }
}

void TableActionHelper::_k_slotRemoveRow()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        table->removeRows(cell.row(), 1);
    }
}

void TableActionHelper::_k_slotRemoveColumn()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        table->removeColumns(cell.column(), 1);
    }
}

void TableActionHelper::_k_slotInsertRowBelow()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        if (cell.row() < table->rows()) {
            table->insertRows(cell.row() + 1, 1);
        } else {
            table->appendRows(1);
        }
    }
}

void TableActionHelper::_k_slotInsertRowAbove()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        table->insertRows(cell.row(), 1);
    }
}

void TableActionHelper::_k_slotInsertColumnBefore()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        table->insertColumns(cell.column(), 1);
    }
}

void TableActionHelper::_k_slotInsertColumnAfter()
{
    QTextTable *table = textCursor().currentTable();
    if (table) {
        const QTextTableCell cell = table->cellAt(textCursor());
        if (cell.column() < table->columns()) {
            table->insertColumns(cell.column() + 1, 1);
        } else {
            table->appendColumns(1);
        }
    }
}

void TableActionHelper::_k_updateActions(bool forceUpdate)
{
    if (forceUpdate) {
        const bool isTable = textCursor().currentTable();
        actionInsertRowBelow->setEnabled(isTable);
        actionInsertRowAbove->setEnabled(isTable);

        actionInsertColumnBefore->setEnabled(isTable);
        actionInsertColumnAfter->setEnabled(isTable);

        actionRemoveRow->setEnabled(isTable);
        actionRemoveColumn->setEnabled(isTable);

        actionRemoveCellContents->setEnabled(isTable);
    }
}

TableActionHelper::TableActionHelper(QObject *parent)
    : QObject(parent)
{
    actionInsertRowBelow = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-below")), i18n("Row Below"), this);
    actionInsertRowBelow->setObjectName("insert_row_below"_L1);
    connect(actionInsertRowBelow, &QAction::triggered, this, [this]() {
        _k_slotInsertRowBelow();
    });

    actionInsertRowAbove = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-above")), i18n("Row Above"), this);
    actionInsertRowAbove->setObjectName("insert_row_above"_L1);
    connect(actionInsertRowAbove, &QAction::triggered, this, [this]() {
        _k_slotInsertRowAbove();
    });

    actionInsertColumnBefore = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-left")), i18n("Column Before"), this);
    actionInsertColumnBefore->setObjectName("insert_column_before"_L1);

    connect(actionInsertColumnBefore, &QAction::triggered, this, [this]() {
        _k_slotInsertColumnBefore();
    });

    actionInsertColumnAfter = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-right")), i18n("Column After"), this);
    actionInsertColumnAfter->setObjectName("insert_column_after"_L1);
    connect(actionInsertColumnAfter, &QAction::triggered, this, [this]() {
        _k_slotInsertColumnAfter();
    });

    actionRemoveRow = new QAction(QIcon::fromTheme(u"edit-table-delete-row"_s), i18n("Row"), this);
    actionRemoveRow->setObjectName("remove_row"_L1);
    connect(actionRemoveRow, &QAction::triggered, this, [this]() {
        _k_slotRemoveRow();
    });

    actionRemoveColumn = new QAction(QIcon::fromTheme(u"edit-table-delete-column"_s), i18n("Column"), this);
    actionRemoveColumn->setObjectName("remove_column"_L1);
    connect(actionRemoveColumn, &QAction::triggered, this, [this]() {
        _k_slotRemoveColumn();
    });

    actionRemoveCellContents = new QAction(QIcon::fromTheme(u"deletecell-symbolic"_s), i18n("Cell Contents"), this);
    actionRemoveCellContents->setObjectName("remove_cell_contents"_L1);
    connect(actionRemoveCellContents, &QAction::triggered, this, [this]() {
        _k_slotRemoveCellContents();
    });
}

TableActionHelper::~TableActionHelper() = default;

QQuickTextDocument *TableActionHelper::document() const
{
    return m_document;
}

void TableActionHelper::setDocument(QQuickTextDocument *document)
{
    if (document == m_document)
        return;

    if (m_document)
        m_document->textDocument()->disconnect(this);

    m_document = document;

    Q_EMIT documentChanged();
}

int TableActionHelper::cursorPosition() const
{
    return m_cursorPosition;
}

void TableActionHelper::setCursorPosition(int position)
{
    if (position == m_cursorPosition)
        return;

    m_cursorPosition = position;
    _k_updateActions(true);

    Q_EMIT cursorPositionChanged();
}

QTextDocument *TableActionHelper::textDocument() const
{
    if (!m_document)
        return nullptr;

    return m_document->textDocument();
}

QTextCursor TableActionHelper::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return QTextCursor();

    QTextCursor cursor = QTextCursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

int TableActionHelper::selectionStart() const
{
    return m_selectionStart;
}

void TableActionHelper::setSelectionStart(int position)
{
    if (position == m_selectionStart)
        return;

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int TableActionHelper::selectionEnd() const
{
    return m_selectionEnd;
}

void TableActionHelper::setSelectionEnd(int position)
{
    if (position == m_selectionEnd)
        return;

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

#include "moc_tableactionhelper.cpp"
