// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SPLITPARAGRAPH_H
#define SPLITPARAGRAPH_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class SplitBlockCommand : public QUndoCommand
{
public:
    SplitBlockCommand(TreeItem *block, const QString &text, int splitIndex, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~SplitBlockCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_parent;
    QString m_text;
    int m_splitIndex;
    int m_row;

    QList<TreeItem *> m_leftBlocks;
    QList<TreeItem *> m_rightBlocks;

    TreeItem *m_originalBlock;
    bool m_ownsOriginalBlock = false;
    bool m_ownsNewBlocks = false;
};

#endif // SPLITPARAGRAPH_H