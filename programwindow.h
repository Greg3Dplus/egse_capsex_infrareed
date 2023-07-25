#ifndef PROGRAMWINDOW_H
#define PROGRAMWINDOW_H

#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "prospect_regs.h"

class ProgramWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgramWindow(quint8 cam_hw_type, QString file_type);
    QString get_exec_path();
    QString get_bitstream_path();
    QString get_exe_default_path();
    QString get_default_bitstream();
    ~ProgramWindow();

private slots:
    void on_browseExe_clicked();

    void on_button_program_clicked();

    void on_browsePdb_clicked();

private:
    quint8 cam_hw_type;
    QString file_type;

    QGridLayout *mainLayout;
    QPushButton *browseExe;
    QPushButton *browsePdb;
    QPushButton *program;

    QLineEdit *pathExe;
    QLineEdit *pathBitstream;

    QLabel *program_sts;
};

#endif // PROGRAMWINDOW_H
