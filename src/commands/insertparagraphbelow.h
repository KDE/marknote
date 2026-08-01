// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INSERTPARAGRAPHBELOW_H
#define INSERTPARAGRAPHBELOW_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class InsertParagraphBelowCommand : public QUndoCommand
{
public:
    InsertParagraphBelowCommand(TreeItem *block, const QString &text, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~InsertParagraphBelowCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_originalBlock;
    TreeItem *m_parent;
    int m_row;
    QString m_text;
    TreeItem *m_newBlock;
};

#endif // INSERTPARAGRAPHBELOW_H
