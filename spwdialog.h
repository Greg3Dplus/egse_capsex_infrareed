#ifndef SPWDIALOG_H
#define SPWDIALOG_H


#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QValidator>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>

#include "print_data.h"
#include "prospect_regs.h"

typedef struct SPW_params {
    quint8 port_num = 1;
    quint16 mult = 2;
    quint16 div  = 8;
    quint8 nb_bytes_per_pixel = 2;
} SPW_params;

class spwdialog : public QDialog
{
    Q_OBJECT

public:
    explicit spwdialog(quint8 cam_fw_type);
    ~spwdialog();

    void set_SPW_params(quint8 port_num, quint16 mult, quint16 div, quint8 nb_bytes_per_pixel);
    SPW_params get_SPW_params();
    void UpdateInfo(void);

private slots:
    void on_SPW_valueChanged();
    void on_OK_clicked();
    void on_cancel_clicked();

private:
  QGridLayout *mainLayout;
  QGridLayout *TMLayout;
  QGroupBox   *TM;
  QSpinBox    *nb_bytes_per_pixel;
  QSpinBox    *SPW_port;
  QSpinBox    *SPW_div;
  QSpinBox    *SPW_mult;
  QLabel      *linkspeed;
  QPushButton *ok;
  QPushButton *cancel;

  SPW_params init_params;  // Initial parameters, when the window is created
};

#endif // SPWDIALOG_H
