// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRANSFORMTOLISTCOMMAND_H
#define TRANSFORMTOLISTCOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class TransformToListCommand : public QUndoCommand
{
public:
    TransformToListCommand(TreeItem *block, bool isOrdered, int startNumber, const QString &text, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~TransformToListCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_originalParent;
    int m_originalRow;
    TreeItem *m_originalBlock;
    bool m_isOrdered;
    int m_startNumber;
    QString m_text;
    QString m_oldText;
    TreeItem *m_outerList;
};

#endif // TRANSFORMTOLISTCOMMAND_H
