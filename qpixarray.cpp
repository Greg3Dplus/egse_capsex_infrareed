#include "qpixarray.h"

QPixArray::QPixArray()
{
    this->ar.clear();
}

void QPixArray::append(quint32 data16) {
    this->ar.append(data16);
}

quint32 QPixArray::at(quint32 idx) {
    return this->ar.at(static_cast<qint32>(idx));
}

void QPixArray::clear(void) {
    this->ar.clear();
}

quint32 QPixArray::size(void) {
    return static_cast<quint32>(this->ar.size());
}

void QPixArray::append(QPixArray pix_data) {
    for (quint32 idx = 0; idx < pix_data.size(); idx++) {
        this->append(pix_data.at(idx));
    }
}

void QPixArray::append(QByteArray pix_data) {
    for (int idx = 0; idx < pix_data.size(); idx++) {
        this->append(static_cast<quint32>(pix_data.at(idx)));
    }
}

void QPixArray::print_data(void) {
    this->print_data(static_cast<int>(this->size()));
}

void QPixArray::print_data(int nb_data) {
    this->print_data(0, nb_data);
}

void QPixArray::print_data(int offset, int nb_data) {
    for (quint32 i = static_cast<quint32>(offset); i < static_cast<quint32>(nb_data); i++) {
        qDebug().noquote()  << "Data " + QString::number(i, 10) + "=0x" + QString::number(this->at(i), 16);
    }
}
