#ifndef CUSTOMSPINBOX_H
#define CUSTOMSPINBOX_H

#include <QSpinBox>
#include <QDebug>
#include <math.h>


class CustomSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    CustomSpinBox();

private:
    void stepBy(qint32 steps);
    QValidator::State validate(QString &input, int &pos) const;
};

#endif // CUSTOMSPINBOX_H
