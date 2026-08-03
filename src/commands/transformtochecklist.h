// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRANSFORMTOCHECKLISTCOMMAND_H
#define TRANSFORMTOCHECKLISTCOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class TransformToChecklistCommand : public QUndoCommand
{
public:
    TransformToChecklistCommand(TreeItem *block, bool isChecked, const QString &text, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~TransformToChecklistCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    bool m_isChecked;
    QString m_text;
    QString m_oldText;

    bool m_wasTaskList;
    bool m_wasChecked;
};

#endif // TRANSFORMTOCHECKLISTCOMMAND_H
