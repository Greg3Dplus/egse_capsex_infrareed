#ifndef CAMERALINK_H
#define CAMERALINK_H
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QThread>
#include <QDir>

// Windows include
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Wrap for MGwin
//#include "mingw.h"
//#include <winsdk_compat.h>
#include "niimaq.h"
#include "prospect_regs.h"
#include "cu_frame.h"

#include "Windows.h"//Sleep for MSVC

//#include "prospect_regs.h"
#define TMTC_SIZE_MAX 32

#define NUM_GRAB_BUFFERS 3

const quint32 CAMERA_LINK_SERIAL_TIMEOUT = 5000;

//const quint32 HEADER_SIZE    = 29*8;

const quint32 FRAME_TIMEOUT_MS = 1000;

const quint8  NB_BITS   = 32;
const quint8  HEADER_EN = 1;

const quint32 CLCTL_TAPNUMBER = 0x02; // ADDRESSE DU REGISTRE

typedef struct Acquisition_params {
    SESSION_ID Sid = 0;
    quint32 Buffsize;

} Acquisition_params;


class CameraLink //public QObject
{
//    Q_OBJECT
public:
    CameraLink();
    ~CameraLink();

    void save_script_file(QString file_name);

    void DisplayIMAQError(Int32 error);
    void CameraLinkTest();
    bool CameraLink_open();
    bool get_Cl_Running();
    void CameraLink_close();
    void CameraLink_write();

    QByteArray send_TM(quint8 nb_data, quint32 addr);
    quint8 send_TM(quint32 addr);
    QByteArray read_data(quint32 nb_data, quint32 addr);

    bool send_TC(quint32 addr, quint8 data);
    bool send_TC(quint8 nb_data, quint32 addr, quint32 data);
    bool send_TC(quint8 nb_data, quint32 addr, const quint8* data);
    bool send_TC(quint32 addr, QByteArray data);
    void write_data(quint32 addr, QByteArray data);

    QByteArray Snap();
    void Grab();
    void ReleaseGrabBuffer();

    // ICD mgmt
    bool ModifyIcd(QString ParameterName, QString Data, bool reload);
    bool ModifyIcd(QString ParameterName, QString Data);
    bool InitIcd();
    void Update_ICD(quint8 TapNumber);
    void Set_Cl_buffer_params(quint32 width, quint32 height, quint8 BinX, quint8 BinY, quint8 TapNumber);

    //high level fct
    void setCl_vid_TimeoutVal(int timeout_ms);
    void UpdateTapNumber(quint8 TapNumber, bool reaload_icd);

    quint32 CL_width;
    quint32 CL_height;
    uInt32 bufferSize;
    uInt32 bufSize;

    uInt32 GetTest();

    // streaming thread parameter transfer
    Acquisition_params Get_CL_Acq_params();

private:
        QStringList GUI_commands;  // Will record all TM/TC commands sent
        bool IsClRunning;
};

#endif // CAMERALINK_H
