// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SPLITLISTITEM_H
#define SPLITLISTITEM_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class SplitListItemCommand : public QUndoCommand
{
public:
    SplitListItemCommand(TreeItem *block, const QString &text, int splitIndex, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~SplitListItemCommand();

    void undo() override;
    void redo() override;

private:
    TreeItem *m_parent;
    int m_row;

    QString m_text;
    int m_splitIndex;

    MDTreeModel *m_model;

    QList<TreeItem *> m_leftBlocks;
    QList<TreeItem *> m_rightBlocks;

    TreeItem *m_originalBlock;
    bool m_ownsOriginalBlock = false;
    bool m_ownsNewBlocks = false;
    bool m_ownsRemovedList = false;
    TreeItem *m_removedList = nullptr;
};

#endif // SPLITLISTITEM_H