#ifndef TEST_MODE_CONFIGURATION_H
#define TEST_MODE_CONFIGURATION_H

#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QGridLayout>
#include <QScrollArea>


class Test_mode_configuration : public QDialog
{
    Q_OBJECT

public:
    explicit Test_mode_configuration(QByteArray regs);
    ~Test_mode_configuration();
    QByteArray get_test_mode_regs_values();

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

