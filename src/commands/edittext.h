// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITTEXT_H
#define EDITTEXT_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class EditTextCommand : public QUndoCommand
{
public:
    EditTextCommand(TreeItem *block,
                    const QString &oldText,
                    const QString &newText,
                    int oldCursorPosition,
                    int newCursorPosition,
                    MDTreeModel *model,
                    QUndoCommand *parent = nullptr);
    ~EditTextCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_block;
    QString m_oldText;
    QString m_newText;
    int m_oldCursorPosition;
    int m_newCursorPosition;
    bool m_isFirstTime;
};

#endif // EDITTEXT_H
