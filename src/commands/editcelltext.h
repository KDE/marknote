// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITCELLTEXT_H
#define EDITCELLTEXT_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class EditCellTextCommand : public QUndoCommand
{
public:
    EditCellTextCommand(TreeItem *block,
                        int row,
                        int column,
                        const QString &oldText,
                        const QString &newText,
                        int oldCursorPosition,
                        int newCursorPosition,
                        MDTreeModel *model,
                        QUndoCommand *parent = nullptr);
    ~EditCellTextCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    int m_row;
    int m_column;
    QString m_oldText;
    QString m_newText;
    int m_oldCursorPosition;
    int m_newCursorPosition;
    MDTreeModel *m_model;
    bool m_isFirstTime;
};

#endif // EDITCELLTEXT_H
