#ifndef QPIXARRAY_H
#define QPIXARRAY_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include "print_data.h"
#include <math.h>

class QPixArray
{
public:
    QPixArray();

    void append(quint32 data16);
    quint32 at(quint32 idx);
    void clear(void);
    quint32 size(void);
    void append(QPixArray pix_data);
    void append(QByteArray pix_data);
    void print_data(void);
    void print_data(int nb_data);
    void print_data(int offset, int nb_data);

private:
    QVector<quint32> ar;
};

#endif // QPIXARRAY_H
