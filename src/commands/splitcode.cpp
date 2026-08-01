// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "splitcode.h"
#include "editcode.h"
#include "insertparagraphbelow.h"

SplitCodeCommand::SplitCodeCommand(TreeItem *block, const QString &oldText, const QString &text, int cursorPosition, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    QString leftText = text.left(cursorPosition);
    QString rightText = text.mid(cursorPosition);

    new EditCodeCommand(block, oldText, leftText, cursorPosition, cursorPosition, model, this);
    new InsertParagraphBelowCommand(block, rightText, model, this);
}

SplitCodeCommand::~SplitCodeCommand()
{
}
