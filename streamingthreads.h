#ifndef STREAMINGTHREADS_H
#define STREAMINGTHREADS_H

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QDataStream>
#include <QIODevice>
#include <QThread>
#include <QDir>
#include <QMutex>
#include <QTime>

#include "mcsdc_registers.h"
#include "cu_frame.h"
#include "rimc_regs.h"
#include "prospect_regs.h"
#include "cmv_registers.h"
#include "mcfmc_registers.h"
//#include "spw_camera.h"
#include "cameralink.h"
#include "niimaq.h"

/**
 * Provides examples of performing a transmit and receive of a packet using
 * the STAR-System C++ API, as described in the application note "Transmitting
 * And Receiving Packets Using the STAR-System C++ API".
 */

#define SPW_HEADER_SIZE  2
#define NB_COPY_BUFFER  128
//#define MAX_WIDTH   2048
//#define MAX_HEIGHT  2048

const quint8 fot_length   = 20;

#define QDEBUG_PRINT_EN 1

typedef struct gui_params {
    bool do_start;
//    SPW_camera spw_camera;
    CameraLink Cl_camera;
    quint8 sensor;
} GUI_params;


class StreamingThreads : public QThread
{
    Q_OBJECT

public:
    explicit StreamingThreads(GUI_params *gui_params, CU_Frame **rcv_data_Byte, QMutex *mutex);
    ~StreamingThreads();
    void run();

    bool compare_ar(QByteArray ar1, QByteArray ar2);
    bool compare_ar(QByteArray ar1, QByteArray ar2, quint32 *idx_ignore, quint32 size_ignore);
    quint32 to_int(quint8 size, QByteArray data);
    quint32 to_int(quint8 size, QByteArray data, quint32 offset);
    void print_log(QString string, QTextStream &out);
    void ReleaseCopyBuffer();

    double get_DDR_bandwidth(quint8 coAdding_val);

private:
    GUI_params* gui_params;

    CU_Frame **rcv_data_Byte;
    QMutex *mutex;
    QTime *time;

signals:
    void FrameReceived();
    void END_SEQUENCE();
    void END_THREAD();
};

#endif // STREAMINGTHREADS_H
