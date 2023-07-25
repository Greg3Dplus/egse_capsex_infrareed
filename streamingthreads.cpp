#include "streamingthreads.h"

    static Int8*        CopyBuffer[NB_COPY_BUFFER];   // copied acquisition buffer


StreamingThreads::StreamingThreads(GUI_params* gui_params, CU_Frame **rcv_data_Byte, QMutex *mutex)
{
    this->gui_params    = gui_params;

    this->rcv_data_Byte = rcv_data_Byte;
    this->mutex = mutex;

}

StreamingThreads::~StreamingThreads()
{

}



void StreamingThreads::run()
{
//    qDebug() << "Inside RUN";
    this->mutex->lock();
    if (this->gui_params->Cl_camera.get_Cl_Running() == 0)
    {
        this->mutex->unlock();
        return;
    }
    this->mutex->unlock();

//    quint16 num_frame = 0;
    this->time = new QTime;
    this->time->start();
    bool EOS = false;
    this->gui_params->Cl_camera.Grab();

//    int t1 = this->time->elapsed();
//    qDebug().noquote() << "#### Time GRAB (ms)" + QString::number(t1,10);
    Acquisition_params CL_Acq_params = this->gui_params->Cl_camera.Get_CL_Acq_params();

    // alloc our copy buffer for storing copy
    for (quint8 i=0; i<NB_COPY_BUFFER; i++)
     CopyBuffer[i] = (Int8*) malloc(CL_Acq_params.Buffsize * sizeof (Int8));
    qDebug().noquote() << " copy Buffsize " + QString::number(CL_Acq_params.Buffsize,10);

    int MaxNbBuffer = int(NB_COPY_BUFFER-2.56*CL_Acq_params.Buffsize/1000000); //
    if (MaxNbBuffer < 25)
        MaxNbBuffer =  25;
    if (MaxNbBuffer > NB_COPY_BUFFER)
       MaxNbBuffer =  NB_COPY_BUFFER;
//    qDebug().noquote() << "Maximum image stored: " + QString::number(MaxNbBuffer,10);

    static uInt32 num_frame = 0;
    static uInt32 actualCopiedBuffer = 0;

    while (! EOS) {
        int result1 = imgGetAttribute (CL_Acq_params.Sid, IMG_ATTR_LAST_VALID_FRAME, &num_frame);
//        this->gui_params->Cl_camera.DisplayIMAQError(result1);
        num_frame++;
//        qDebug().noquote() << "COPY BUFFER Acquisition  IN PROGRESS...." ;
//       qDebug().noquote() << "num_frame " + QString::number(num_frame,10);

//        int t2 = this->time->elapsed();
//        qDebug().noquote() << "#### Time before CopyBuffer (ms)" + QString::number(t2-t1,10);

        // Copy the last valid buffer
        int result = imgSessionCopyBufferByNumber(CL_Acq_params.Sid, num_frame, CopyBuffer[(num_frame) % MaxNbBuffer], IMG_OVERWRITE_GET_NEWEST, &actualCopiedBuffer, NULL);


//        int t3 = this->time->elapsed();
//        qDebug().noquote() << "#### Time AFTER CopyBuffer (ms)" + QString::number(t3-t2,10);

        qDebug().noquote() << " copy actualCopiedBuffer " + QString::number(actualCopiedBuffer,10);

        if (result == 0)
          qDebug().noquote() << "Acquisition Frame " + QString::number(num_frame,10) + " : OK";
        else {
          qDebug().noquote() << "Acquisition Frame " + QString::number(num_frame,10) + " : NOT OK";
          emit END_SEQUENCE();
          emit END_THREAD();
        }

//        this->gui_params->Cl_camera.DisplayIMAQError(result);


//         int t4 = this->time->elapsed();
//         qDebug().noquote() << "#### Time AFTER QbyteArray (ms)" + QString::number(t4-t3,10);


        quint32 NUM_PRINT = 64;
        quint32 nb_data   = CL_Acq_params.Buffsize;
//        qDebug().noquote() << "nb data = " + QString::number(nb_data);

//            for (quint32 i = 0 ; i < NUM_PRINT ; i++)
//                qDebug().noquote() << "Frame "+ QString::number(num_frame) +" pixel " + QString::number(i) + ": 0x" + QString::number(CopyBuffer[i] & 0xff,16);

//        for (quint32 i = 0 ; i < NUM_PRINT ; i++)
//            qDebug().noquote() << "End Frame "+ QString::number(num_frame) +" pixel " + QString::number(nb_data - (NUM_PRINT-i)) + ": 0x" + QString::number(CopyBuffer[nb_data - (NUM_PRINT-i)] & 0xff,16);


        this->mutex->lock();
       if ((*this->rcv_data_Byte)->init_pkt(CopyBuffer[(num_frame) % MaxNbBuffer])) {
            emit FrameReceived();
//            qDebug().noquote() << "ID  " + QString::number((*this->rcv_data_Byte)->getFrame_ID(),10);
           if ((*this->rcv_data_Byte)->isEOS()) {
                EOS = true;
//               qDebug().noquote() << "is EOS!";
            }
        }
        else {
            EOS = true;
        }
      this->mutex->unlock();
    }

    qDebug().noquote() << "Received "+ QString::number(num_frame+1,10) + " frames";

    quint8 CMV_STS = this->gui_params->Cl_camera.send_TM(SENS_CTL_ba | CMVCTL_STS_AD);
    if ((CMV_STS & CMVCTL_STS_AHBERR_gm) != 0) {
        qDebug().noquote()  << "ERROR: SENS_STS AHBERR bit is asserted; 0x0" <<QString::number(CMV_STS,16);

    }


    //Release Buffer
    this->gui_params->Cl_camera.ReleaseGrabBuffer();


     emit END_SEQUENCE();


     emit END_THREAD();

}

void StreamingThreads::ReleaseCopyBuffer(){

     for (quint8 i=0; i<NB_COPY_BUFFER; i++)
         free(CopyBuffer[i]);

}

bool StreamingThreads::compare_ar(QByteArray ar1, QByteArray ar2) {
    quint32* idx_ignore = {};
    return compare_ar(ar1, ar2, idx_ignore, 0);
}

bool StreamingThreads::compare_ar(QByteArray ar1, QByteArray ar2, quint32* idx_ignore, quint32 size_ignore) {
    bool ret = true;

    if (ar1.size() != ar2.size()) {
        qDebug () << "ERROR: Size differs: Size 1 = " << ar1.size() << ", size2 = " << ar2.size();
        return false;
    }

    for (qint32 i = 0; i < ar1.size(); i++) {
        bool to_ignore = false;
        for (quint32 j = 0; j < size_ignore; j++) {
            if (idx_ignore[j] == static_cast<quint32>(i)) {
                to_ignore = true;
                //qDebug () << "Index " + QString::number(i, 10) + " ignored";
            }
        }

        if (ar1.at(i) != ar2.at(i) && to_ignore == false) {
            qDebug () << "Data " + QString::number(i) + " differs: Data1 = 0x" + QString::number(static_cast<quint8>(ar1.at(i)), 16) +
                         ", Data2 = 0x" + QString::number(static_cast<quint8>(ar2.at(i)), 16);
            ret = false;
        }
    }

    return ret;
}

quint32 StreamingThreads::to_int(quint8 size, QByteArray data) {
    return to_int(size, data, 0);
}

quint32 StreamingThreads::to_int(quint8 size, QByteArray data, quint32 offset) {
    quint32 value = 0;

    for (quint32 i = offset; i < offset + size; i++) {
        value |= static_cast<quint32>(data.at(static_cast<qint32>(i)) << (8*i));
    }

    return value;
}

void StreamingThreads::print_log(QString string,  QTextStream &out) {
    qDebug().noquote()  << string;
    out << string + "\n";
}

// Return bandwidth in MPixels/s
double StreamingThreads::get_DDR_bandwidth(quint8 coAdding_val) {
    //return 4 * SysClk_freq;
    if (coAdding_val == 1) {
        // return 1024; // 65% of 512
        return 665;
    }
    else {
        // return 512;// 65% of 256
         return 332;
    }
}
