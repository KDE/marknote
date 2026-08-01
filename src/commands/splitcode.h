// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SPLITCODE_H
#define SPLITCODE_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class SplitCodeCommand : public QUndoCommand
{
public:
    SplitCodeCommand(TreeItem *block, const QString &oldText, const QString &text, int cursorPosition, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~SplitCodeCommand();
};

#endif // SPLITCODE_H
