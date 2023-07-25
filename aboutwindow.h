#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

class aboutWindow : public QDialog
{
    Q_OBJECT

public:
    explicit aboutWindow(quint32 egse_version);
    ~aboutWindow();

    void set_fpga_version(QString project_name, quint32 fpga_version);

private slots:
    void on_OK_clicked();

private:
  QGridLayout *mainLayout;
  QLabel *fpga_version_lbl;
  QPushButton *ok;
};

#endif // ABOUTWINDOW_H
