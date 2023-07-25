#ifndef CMV_READ_REGISTERS_H
#define CMV_READ_REGISTERS_H

#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QGridLayout>
#include <QScrollArea>


class CMV_read_registers : public QDialog
{
    Q_OBJECT

public:
    explicit CMV_read_registers(QByteArray cmv_regs);
    ~CMV_read_registers();
    QByteArray get_cmv_regs_values();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_canceled();

private:
    QList<QLabel *> reg_names;
    QList<QSpinBox *> reg_vals;
    QGridLayout *mainLayout;
    QScrollArea *scrollArea;
};

#endif // CMV_READ_REGISTERS_H
