// SPDX-FileCopyrightText: Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sketchhistory.h"

bool HistoryController::isUndoAvailable() const
{
    if (history.size() == 0) {
        return false;
    }
    if (at0th) {
        return false;
    }
    return current < history.end() && current >= history.begin();
}

bool HistoryController::isRedoAvailable() const
{
    if (at0th) {
        return true;
    }
    if (history.size() == 0) {
        return false;
    }
    if (current == history.end() - 1) {
        return false;
    }
    return current < history.end() - 1 && current >= history.begin();
}

void HistoryController::add_stroke(const QList<QVector2D> &points, const QString &color, float &width, bool &isEraser)
{
    Stroke stroke{
        .points = points,
        .color = color,
        .width = width,
        .isEraser = isEraser,
    };
    history.push_back(stroke);
}

void HistoryController::submitStroke(const QList<QVector2D> &points, const QString &color, float width, bool isEraser)
{
    if (at0th) {
        history.clear();
        at0th = false;
        add_stroke(points, color, width, isEraser);
        current = history.begin();
    } else if (history.size() == 0) {
        // allow and assign current iterator
        add_stroke(points, color, width, isEraser);
        current = history.begin();
    } else if (current < history.end() - 1) {
        // truncate the history after that point
        history.erase(current + 1, history.end());
        add_stroke(points, color, width, isEraser);
        current = history.end() - 1;
    } else if (current == history.end() - 1) {
        add_stroke(points, color, width, isEraser);
        current = history.end() - 1;
    } else {
        return;
    }
    Q_EMIT undoAvailableChanged();
    Q_EMIT redoAvailableChanged();
}

void HistoryController::undoStroke()
{
    // this check should also be made by the caller, but is also made here to add redundancy
    if (isUndoAvailable()) {
        if (current == history.begin()) {
            at0th = true;
        } else {
            current--;
        }
    }
    Q_EMIT undoAvailableChanged();
    Q_EMIT redoAvailableChanged();
}

Stroke HistoryController::redoStroke()
{
    // history can't be empty when on redoStroke
    if (history.empty()) {
        return Stroke{};
    }
    // as return type is fixed, here we can't make redundant availability check, so make sure to call this only after checking
    if (at0th) {
        at0th = false;
    } else {
        current++;
    }
    Q_EMIT undoAvailableChanged();
    Q_EMIT redoAvailableChanged();
    return *current;
}

void HistoryController::reset()
{
    history.clear();
    current = history.end();
    at0th = false;
    Q_EMIT undoAvailableChanged();
    Q_EMIT redoAvailableChanged();
}