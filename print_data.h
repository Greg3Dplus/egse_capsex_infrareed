#ifndef PRINT_DATA_H
#define PRINT_DATA_H

#include <QByteArray>
#include <QDebug>
#include <QString>

#include <math.h>

void print_data_Bytes(const quint8 *data, int size);
void print_data_Bytes(QByteArray data, int size);
void print_data_Bytes(QByteArray data, int size, int offset);
void print_data_Bytes(QByteArray data);

quint32 log2_int(quint32 input_val);

#endif // PRINT_DATA_H
