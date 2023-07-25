#ifndef CLDIALOG_H
#define CLDIALOG_H


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
#include <QComboBox>

#include "print_data.h"
#include "prospect_regs.h"
#include "cameralink.h"

typedef struct CL_params {
    quint8 Port_config = 0;
    quint8 TapNumber = 1;

} CL_params;

class CLdialog : public QDialog
{
    Q_OBJECT

public:
    explicit CLdialog(quint8 TapNumber);
    ~CLdialog();

//    void set_SPW_params(quint8 port_num, quint16 mult, quint16 div, quint8 nb_bytes_per_pixel);
//    CL_params get_SPW_params();
//    void UpdateInfo(void);
    CL_params Get_Cl_params();
    void Set_Cl_params( quint8 TapNumber);


private slots:
    void on_OK_clicked();
    void on_cancel_clicked();


private:

  QGridLayout *mainLayout;
  QLabel      *config_port_lbl;
  QLabel      *cl_serial_baud_lbl;

  QSpinBox   *SpinboxTest;
  QComboBox  *tap_number;
  QLabel     *tap_number_lbl;

  QPushButton *ok;
  QPushButton *cancel;

  quint8 TapNumber;

  CL_params init_params;// Initial parameters, when the window is created

};

#endif // SPWDIALOG_H
