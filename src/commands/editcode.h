// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITCODE_H
#define EDITCODE_H

#include "mdtreemodel/mdtreemodel.h"
#include <QUndoCommand>

class EditCodeCommand : public QUndoCommand
{
public:
    EditCodeCommand(TreeItem *block,
                    const QString &oldText,
                    const QString &newText,
                    int oldCursorPosition,
                    int newCursorPosition,
                    const QString &m_oldLanguage,
                    const QString &m_newLanguage,
                    MDTreeModel *model,
                    QUndoCommand *parent = nullptr);
    ~EditCodeCommand();

    void undo() override;
    void redo() override;

private:
    MDTreeModel *m_model;
    TreeItem *m_block;
    QString m_oldText;
    QString m_newText;
    int m_oldCursorPosition;
    int m_newCursorPosition;
    QString m_oldLanguage;
    QString m_newLanguage;
    bool m_isFirstTime;
};

#endif // EDITCODE_H
