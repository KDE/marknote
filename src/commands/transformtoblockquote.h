// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRANSFORMTOBLOCKQUOTECOMMAND_H
#define TRANSFORMTOBLOCKQUOTECOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class TransformToBlockquoteCommand : public QUndoCommand
{
public:
    TransformToBlockquoteCommand(TreeItem *block, int level, const QString &text, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~TransformToBlockquoteCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_originalParent;
    int m_originalRow;
    TreeItem *m_originalBlock;
    int m_level;
    QString m_text;
    QString m_oldText;
    TreeItem *m_outerBlockquote;
};

#endif // TRANSFORMTOBLOCKQUOTECOMMAND_H
