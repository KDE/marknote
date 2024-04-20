#ifndef LATEXTOPNG_H
#define LATEXTOPNG_H

#include <QtQml>

class LatexToPNG : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LatexToPNG(QObject *parent = nullptr);
    Q_INVOKABLE void renderPNG(QString latexText);
};

#endif // LATEXTOPNG_H
