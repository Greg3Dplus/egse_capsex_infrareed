#include "print_data.h"

void print_data_Bytes(const quint8* data, int size) {
    for (int i =0; i < size; i++) {
        qDebug().noquote()  << "Byte " + QString::number(i, 10) + "=" + QString::number(data[i],16);
    }
}

void print_data_Bytes(QByteArray data, int size) {
    print_data_Bytes(data, size, 0);
}

void print_data_Bytes(QByteArray data, int size, int offset) {
    for (int i =offset; i < size; i++) {
        qDebug().noquote()  << "Byte " + QString::number(i, 10) + "=0x" + QString::number(static_cast<quint8>(data.at(i)), 16);
    }
}

void print_data_Bytes(QByteArray data) {
    print_data_Bytes(data, data.size());
}

quint32 log2_int(quint32 input_val) {
    double log2_fl = log2(input_val);
    quint32 log2_int = static_cast<quint32>(log2_fl);
    if (log2_fl - static_cast<qint32>(log2_fl) != 0.0) {
        log2_int++;
    }

    return log2_int;
}
