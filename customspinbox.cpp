#include "customspinbox.h"

CustomSpinBox::CustomSpinBox()
{

}

void CustomSpinBox::stepBy(qint32 steps)
{
    qint32 actualValue = this->value();
    switch(steps) {
    case 1 :    setValue(actualValue<<1);
        break;
    case -1 :   setValue(actualValue>>1);
        break;
    default:    QSpinBox::stepBy(steps);
        break;
    }
}

QValidator::State CustomSpinBox::validate(QString &input, int &pos) const
{
    //qDebug() << input;
    //qDebug() << pos;
    qint32 input_int = input.toInt();
    double input_log = log2(input_int);
    if (input_log - static_cast<qint32>(input_log) != 0.0)
        //if (input_int != 1 && input_int != 2 && input_int != 4 && input_int != 8)
        return QValidator::State::Intermediate;

    return QValidator::State::Acceptable;
}
