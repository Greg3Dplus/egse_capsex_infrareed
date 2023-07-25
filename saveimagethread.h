#ifndef SAVEIMAGETHREAD_H
#define SAVEIMAGETHREAD_H

#include <QThread>

#include "cu_frame.h"

class saveImageThread : public QThread
{
    Q_OBJECT

public:
    explicit saveImageThread();
    void run (void);

    bool add_cu_frame(CU_Frame *CU_frame);

    bool add_cu_frame(CU_Frame *CU_frame, QString Name, bool script);
    void set_stop(bool st);
    void reset(quint32 num_sequence, double SENS_in_freq, quint16 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 nb_image, quint8 fot_length, quint8 cam_fw_type, double H_Period);
private:
    quint8 nb_read;
    quint8 nb_write;
    bool full_empty;
    CU_Frame *CU_FRAME_tab[256];
    QString name_tab[256];
    bool stop;
    quint32 num_image_txt;
    quint32 num_sequence;
    double SENS_in_freq;
    quint16 ADC_gain;
    quint8 vramp1;
    quint8 vramp2;
    quint16 offset;
    quint8 nb_image;
    quint8 fot_length;
    quint8 cam_fw_type;
    double H_Period;

signals:
    void NEW_SAVE(QString file_name);
    void END_THREAD();

};

#endif // SAVEIMAGETHREAD_H
