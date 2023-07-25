#ifndef SCRIPT_H
#define SCRIPT_H

#include <QPushButton>
#include <QGridLayout>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QDebug>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QProgressBar>
#include <QProcess>

//#include <spw_camera.h>
#include <cameralink.h>
#include "prospect_regs.h"
#include "cmv_registers.h"
#include "cu_frame.h"
#include "imx.h"
#include "cam_controller.h"
#include "cameralink.h"
#include "streamingthreads.h"
#include "saveimagethread.h"
#include "saveBMPthread.h"

class script : public QDialog
{
    Q_OBJECT

public:
    script(CAM_controller *cam_controller, IMX *imx, CameraLink *Cl_camera, QString path, quint8 cam_fw_type, quint8 SensorIsColor, bool batch_mode);
    void newScript(void);
    void run_script();

private:
    QGridLayout *Layout;
    QLabel      *file;
    QPushButton *start;
    QPushButton *close_cancel;
    QPushButton *open_log;
    QPushButton *browse;

    QProgressBar *progressBar;

//    StreamingThreads *strThread;
    GUI_params gui_params;
    CU_Frame *rcv_data_Byte;


    QString path;

//    SPW_camera *spw_camera;
    CameraLink *Cl_camera;
    CAM_controller *cam_controller;
    IMX  *imx;
    quint8 cam_fw_type;
    quint8 cam_type;
    quint8 SensorIsColor;

    quint8 TapNumber;
    bool batch_mode;

    saveImageThread *saveThread;
    saveBMPThread *save_BMP_Thread;

private slots:

    void on_start_clicked();
    void on_close_cancel_clicked();
    void on_open_log_clicked();
    void on_browse_script_clicked();
    void EndThread(void);

signals:
    void WRITE(quint32 add, QByteArray data);
    void READ (quint32 add, quint32 nb_data, QByteArray ref_data);

};

#endif // SCRIPT_H
