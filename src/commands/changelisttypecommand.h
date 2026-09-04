// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHANGELISTTYPECOMMAND_H
#define CHANGELISTTYPECOMMAND_H

#include "mdoptions/mdoptions.h"
#include "mdtreemodel/mdtreemodel.h"
#include <QList>
#include <QUndoCommand>

class ChangeListTypeCommand : public QUndoCommand
{
public:
    ChangeListTypeCommand(TreeItem *block, MDOptions::ListType targetType, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~ChangeListTypeCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_block;
    MDTreeModel *m_model;
    MDOptions::ListType m_targetType;
    TreeItem *m_listNode;

    struct OriginalState {
        TreeItem *listItem;
        MD::ListItem::ListType listType;
        bool isTaskList;
    };
    QList<OriginalState> m_originalStates;
    bool m_isValid;
};

#endif // CHANGELISTTYPECOMMAND_H
