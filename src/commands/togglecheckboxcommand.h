// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOGGLECHECKBOXCOMMAND_H
#define TOGGLECHECKBOXCOMMAND_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class ToggleCheckboxCommand : public QUndoCommand
{
public:
    ToggleCheckboxCommand(TreeItem *block, MDTreeModel *model, QUndoCommand *parent = nullptr);
    ~ToggleCheckboxCommand() override;

    void undo() override;
    void redo() override;

private:
    TreeItem *m_listItemNode;
    MDTreeModel *m_model;
    bool m_wasChecked;
    bool m_isChecked;
    bool m_isValid;
};

#endif // TOGGLECHECKBOXCOMMAND_H
