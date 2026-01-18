// SPDX-FileCopyrightText: Siddharth Chopra <contact.sid.chopra@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MARKNOTE_SKETCHUNDO_H
#define MARKNOTE_SKETCHUNDO_H

#include <QObject>
#include <QString>
#include <QVector2D>
#include <deque>
#include <qqmlintegration.h>

struct Stroke {
    Q_GADGET
    Q_PROPERTY(QList<QVector2D> points MEMBER points)
    Q_PROPERTY(QString color MEMBER color)
    Q_PROPERTY(float width MEMBER width)
    Q_PROPERTY(bool isEraser MEMBER isEraser)
public:
    QList<QVector2D> points;
    QString color;
    float width = 0;
    bool isEraser = false;
};
Q_DECLARE_METATYPE(Stroke)

class HistoryController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool undoAvailable READ isUndoAvailable NOTIFY undoAvailableChanged FINAL)
    Q_PROPERTY(bool redoAvailable READ isRedoAvailable NOTIFY redoAvailableChanged FINAL)

    std::deque<Stroke> history;
    std::deque<Stroke>::iterator current = history.end();
    bool at0th = false;

    void add_stroke(const QList<QVector2D> &points, const QString &color, float &width, bool &isEraser);

public:
    HistoryController() = default;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    Q_INVOKABLE void submitStroke(const QList<QVector2D> &points, const QString &color, float width, bool isEraser);
    Q_INVOKABLE void undoStroke();
    Q_INVOKABLE Stroke redoStroke();
    Q_INVOKABLE void reset();

Q_SIGNALS:
    void undoAvailableChanged();
    void redoAvailableChanged();
};

#endif // MARKNOTE_SKETCHUNDO_H
