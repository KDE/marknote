// SPDX-FileCopyrightText: 2026 Prayag Jain <prayagjain2@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "removeblockscommand.h"

RemoveBlocksCommand::RemoveBlocksCommand(const QList<TreeItem *> &blocks, MDTreeModel *model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_blocksToRemove(blocks)
    , m_firstTime(true)
{
}

RemoveBlocksCommand::~RemoveBlocksCommand()
{
}

void RemoveBlocksCommand::redo()
{
    if (m_firstTime) {
        m_firstTime = false;

        QList<TreeItem *> topLevelBlocks;
        for (TreeItem *block : m_blocksToRemove) {
            bool hasAncestor = false;
            for (TreeItem *other : m_blocksToRemove) {
                if (block != other && block->isDescendantOf(other)) {
                    hasAncestor = true;
                    break;
                }
            }
            if (!hasAncestor && !topLevelBlocks.contains(block)) {
                topLevelBlocks.append(block);
            }
        }

        m_model->clearSelection();

        TreeItem *focused = m_model->focusedBlock();
        if (focused) {
            for (TreeItem *b : topLevelBlocks) {
                if (focused == b || focused->isDescendantOf(b)) {
                    m_model->clearFocus();
                    break;
                }
            }
        }

        for (TreeItem *block : topLevelBlocks) {
            TreeItem *current = block;
            while (current && current->parent()) {
                TreeItem *parent = current->parent();
                int index = current->row();

                Removal r;
                r.item = m_model->takeItem(parent, index);
                r.parent = parent;
                r.index = index;
                m_removals.append(r);

                if (parent->type() == MDOptions::ElementType::List) {
                    m_model->updateListNumbers(parent, false);
                }

                if (parent->childCount() == 0 && parent->parent()) {
                    current = parent;
                } else {
                    break;
                }
            }
        }
    } else {
        m_model->clearSelection();
        for (int i = 0; i < m_removals.size(); ++i) {
            const Removal &r = m_removals[i];
            m_model->takeItem(r.parent, r.index);
            if (r.parent->type() == MDOptions::ElementType::List) {
                m_model->updateListNumbers(r.parent, false);
            }
        }
    }
}

void RemoveBlocksCommand::undo()
{
    for (int i = m_removals.size() - 1; i >= 0; --i) {
        const Removal &r = m_removals[i];
        m_model->insertItem(r.parent, r.index, r.item);
        if (r.parent->type() == MDOptions::ElementType::List) {
            m_model->updateListNumbers(r.parent, false);
        }
    }
}
