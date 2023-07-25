#ifndef IMX_READ_REGISTERS_H
#define IMX_READ_REGISTERS_H

#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QByteArray>


class IMX_read_registers : public QDialog
{
    Q_OBJECT

public:
    explicit IMX_read_registers(QByteArray IMX_regs);
    ~IMX_read_registers();
    QByteArray get_imx_regs_values();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_canceled();

private:
    QList<QLabel *> reg_names;
    QList<QSpinBox *> reg_vals;
    QGridLayout *mainLayout;
    QScrollArea *scrollArea;
};

#endif // IMX_READ_REGISTERS_H
