#include "script.h"

    static Int8*        CopyBuffer[NB_COPY_BUFFER];

script::script(CAM_controller *cam_controller, IMX *imx,  CameraLink *Cl_camera,  QString path, quint8 cam_fw_type, quint8 SensorIsColor, bool batch_mode) : QDialog()
{

    if (batch_mode)
        this->path = path;
    this->Cl_camera = Cl_camera;
    this->cam_controller = cam_controller;
    this->imx = imx;
    this->cam_fw_type = cam_fw_type;
    if (this->cam_fw_type == CAM_FW_CINEMA)
        this->cam_type = 0;
    else
        this->cam_type = 1;
    this->SensorIsColor = SensorIsColor;
    this->batch_mode = batch_mode;

    this->Layout = new QGridLayout();
    this->file = new QLabel("Script: ");

    this->start  = new QPushButton("Start");
    this->start->setDisabled(true);
    QObject::connect(this->start,     SIGNAL(clicked()), this, SLOT(on_start_clicked()));
    this->close_cancel = new QPushButton("Close");
    QObject::connect(this->close_cancel,     SIGNAL(clicked()), this, SLOT(on_close_cancel_clicked()));
    this->open_log  = new QPushButton("Open log");
    this->open_log->setVisible(false);
    QObject::connect(this->open_log,     SIGNAL(clicked()), this, SLOT(on_open_log_clicked()));
    this->browse = new QPushButton("Browse");
    this->browse->setVisible(true);
    QObject::connect(this->browse,     SIGNAL(clicked()), this, SLOT(on_browse_script_clicked()));

    this->progressBar = new QProgressBar;
    this->progressBar->setValue(0);

    this->Layout->addWidget(file, 0, 0, 1, 3);
    this->Layout->addWidget(this->progressBar, 1, 0, 1, 3);
    this->Layout->addWidget(this->browse, 2, 0);
    this->Layout->addWidget(this->start, 2, 1);
    this->Layout->addWidget(this->close_cancel, 2, 2);
    this->Layout->addWidget(this->open_log, 2, 3);

    setLayout(this->Layout);
    setWindowTitle("Script");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->saveThread = new saveImageThread();
    this->save_BMP_Thread = new saveBMPThread();
    connect(this->saveThread,       &saveImageThread::END_THREAD , this, &script::EndThread);
    connect(this->save_BMP_Thread,  &saveBMPThread::END_THREAD , this, &script::EndThread);


}

void script::run_script() {

    qDebug().noquote() << "Running script :" << this->path;

    qint32 nb_line = 0;
    QFile script_file(this->path);
    QTextStream flux(&script_file);
    QString line;
    QStringList lineSplit;
    quint32 add;
    qint32 base = 16;
    bool save_txt = false;
    bool save_bmp = false;

    QStringList cmd_list;
    cmd_list.append("BASE");
    cmd_list.append("CONFIG_CL");
    cmd_list.append("SLEEP");
    cmd_list.append("WRITE");
    cmd_list.append("READ");
    cmd_list.append("CONF_FILE");
    cmd_list.append("RQST_VIDEO");  // This command includes the TC, the TM and the reception of the video packets
    cmd_list.append("RCV_VIDEO");   // This command includexs only reception of the video packets
    cmd_list.append("CONF_NB_BITS");
    cmd_list.append("CREATE_REF");
    cmd_list.append("CONF_HEIGHT");
    cmd_list.append("PRINT");
    cmd_list.append("CLOSE_CL");
    if(script_file.open(QIODevice::ReadOnly))
    {
        QFile log_file(this->path + ".log");
        log_file.open(QIODevice::WriteOnly);
        QTextStream log_flux(&log_file);

        QString err_msg = "";

        while(!flux.atEnd())
        {
            line = flux.readLine();
            nb_line ++;
            QString line_str (" at line "+ QString::number(nb_line) + "\n");

            lineSplit = line.split(QRegExp("\\s+"));

            if (lineSplit.at(0).toUpper() == "BASE") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
                else if (lineSplit.at(1).toUInt(nullptr, 10) != 2 &&
                         lineSplit.at(1).toUInt(nullptr, 10) != 10 &&
                         lineSplit.at(1).toUInt(nullptr, 10) != 16)
                    err_msg.append("Parsing ERROR" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "CONFIG_CL") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "SLEEP") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "WRITE") {
                if (lineSplit.size() < 3)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "READ") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "CONF_FILE") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
                else {
                    for(qint8 j = 1; j < lineSplit.size(); ++j) {
                        if (lineSplit.at(j).toLower() != "bmp" && lineSplit.at(j) != "txt")
                            err_msg.append("Unknown Argument " + lineSplit.at(j) + line_str);
                    }
                }
            }
            else if (lineSplit.at(0).toUpper() == "RQST_VIDEO") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
                else {
                    nb_line += static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));
                }
            }
            else if (lineSplit.at(0).toUpper() == "RCV_VIDEO") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
                else {
                    nb_line += static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));
                }
            }
            else if (lineSplit.at(0).toUpper() == "CONF_NB_BITS") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "CREATE_REF") {
                if (lineSplit.size() < 19)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "CONF_HEIGHT") {
                if (lineSplit.size() < 2)
                    err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "PRINT") {
               if (lineSplit.size() < 2)
                   err_msg.append("Not enough arguments" + line_str);
            }
            else if (lineSplit.at(0).toUpper() == "CLOSE_CL") {
                if (lineSplit.size() > 2)
                    err_msg.append("Too much arguments" + line_str);
            }
            else if (lineSplit.at(0).left(1) != "" && lineSplit.at(0).left(1) != "#") {
                err_msg.append("UNKNOWN command" + line_str);
            }

        }

        if (err_msg != "") {
            qDebug().noquote() << err_msg;
            log_flux << err_msg;

        }
        else {
            quint8 nb_bits = 10;
            quint16 ref_height = 2048;
            CU_Frame *ref_frame;
            quint16 pix_start = 0;
            quint16 pix_last = 2047;
            quint16 row_start = 0;
            quint16 nb_row = 2048;

            quint16 black_lim = 0;
            quint16 white_lim = 0;

            quint8 nb_bytes = 2;

            QByteArray frame_tag;

            quint8 footer = 0;
            quint8 binning_X = 1;
            quint8 binning_Y = 1;
            quint8 nb_bits_sensor = 10;
            quint8 test_mode = 0;
            quint32 integration_time = 0;
            quint8 binmode = 0;
            quint8 nb_chan = 2;
            quint8 ref_ADC_gain    = 0;
            quint8 frame_idx_start = 0;
            quint8 EOS_last        = 1;
            bool compare = false;

            flux.seek(0);
            qint32 line_read = 0;
            for (qint32 i=0; i<nb_line; ++i)
            {
                line_read++;
                line = flux.readLine();

                int idx = line.indexOf("#");
                if (idx != -1) {
                    line.truncate(idx-1);
                }

                lineSplit = line.split(QRegExp("\\s+"));

                // Remove empty/null strings from the line
                lineSplit.removeAll(QString(""));

                if (lineSplit.size() > 0) {
                    if (cmd_list.contains(lineSplit.at(0).toUpper())) {
                        log_flux << "Start command: " << lineSplit.join(" ") << "\n";
                    }
                }

                if (lineSplit.size()>=2)
                {
                    if (lineSplit.at(0).toUpper() == "CONFIG_CL") { // a changer en config CL
                        qDebug().noquote() << "Configure CameraLink interface ...";

                        this->Cl_camera->CameraLink_open();
                        this->TapNumber = 1;
                        this->TapNumber = static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));

                        if (this->TapNumber > 1){

                            this->TapNumber = 2;
                        }else {
                             this->TapNumber = 1;
                        }

                        this->Cl_camera->send_TC((MC_CL_ba | CLCTL_TAPNUMBER),this->TapNumber);
                        // update ICD

                        this->Cl_camera->UpdateTapNumber(this->TapNumber, false);
                        this->Cl_camera->Update_ICD(this->TapNumber);

                    }
                    else if (lineSplit.at(0).toUpper() == "BASE")
                    {
                        base = lineSplit.at(1).toInt(nullptr, 10);
                        qDebug().noquote()  << "Setting base to" << base;
                    }

                    else if (lineSplit.at(0).toUpper() == "SLEEP")
                    {
                        qint32 sleep_ms = lineSplit.at(1).toInt(nullptr, 10);
                        qDebug().noquote()  << "Sleep for " + QString::number(sleep_ms,10) + " ms";
                        Sleep(unsigned(sleep_ms));
                    }

                    else if (lineSplit.at(0).toUpper() == "READ")
                    {
                        QByteArray ref_data;

                        add = lineSplit.at(1).toUInt(nullptr, base);

                        quint32 nb_data = lineSplit.at(2).toUInt(nullptr, 10);
                        if (lineSplit.size()>3)
                            for (qint32 i = 3; i<lineSplit.size(); ++i)
                            {
                                ref_data.append(static_cast<qint8>(lineSplit.at(i).toInt(nullptr, base)));

                            }
                        QByteArray rd_data = this->Cl_camera->read_data(nb_data, add);

                        for(quint32 num_data = 0; num_data < nb_data; ++num_data) {
                            if (static_cast<quint32>(ref_data.size()) > num_data) {
                                if (rd_data.at(static_cast<qint32>(num_data)) != ref_data.at(static_cast<qint32>(num_data))) {
                                    log_flux << "  ERROR: Data " << num_data << ". Received " + QString::number(static_cast<quint8>(rd_data.at(static_cast<qint32>(num_data))), base) + ", expected " +
                                                QString::number(static_cast<quint8>(ref_data.at(static_cast<qint32>(num_data))), base) << "\n";

                                    qDebug().noquote() << "  ERROR: Data " << num_data << ". Received " + QString::number(static_cast<quint8>(rd_data.at(static_cast<qint32>(num_data))), base) + ", expected " +
                                                          QString::number(static_cast<quint8>(ref_data.at(static_cast<qint32>(num_data))), base) + " (Read Address " + QString::number(add,base) + ", line " + QString::number(i, 10) + ")";
                                }
                            }
                        }
                    }
                    else if (lineSplit.at(0).toUpper() == "WRITE")
                    {
                        QByteArray data;

                        add = lineSplit.at(1).toUInt(nullptr, base);
                        if (lineSplit.size()>2)
                        {
                            for (qint32 i = 2; i<lineSplit.size(); ++i)
                            {
                                data.append(static_cast<qint8>(lineSplit.at(i).toInt(nullptr, base)));
                            }
                        }

                        this->Cl_camera->write_data(add, data);
                        // Sleep(100);
                    }
                    else if (lineSplit.at(0).toUpper() == "CONF_FILE") {

                        save_bmp = false;
                        save_txt = false;
                        for(qint8 j = 1; j < lineSplit.size(); ++j) {
                            if (lineSplit.at(j).toLower() == "bmp")
                                save_bmp = true;
                            else
                                save_txt = true;
                        }

                    }
                    else if (lineSplit.at(0).toUpper() == "CREATE_REF") {
                        pix_start = static_cast<quint16>(lineSplit.at(1).toUInt(nullptr, 10));
                        pix_last = static_cast<quint16>(lineSplit.at(2).toUInt(nullptr, 10));
                        row_start = static_cast<quint16>(lineSplit.at(3).toUInt(nullptr, 10));
                        nb_row = static_cast<quint16>(lineSplit.at(4).toUInt(nullptr, 10));

                        black_lim = static_cast<quint16>(lineSplit.at(5).toUInt(nullptr, 10));
                        white_lim = static_cast<quint16>(lineSplit.at(6).toUInt(nullptr, 10));

                        nb_bytes = static_cast<quint8>(lineSplit.at(7).toUInt(nullptr, 10));

                        //QByteArray frame_tag = static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));
                        frame_tag.clear();
                        frame_tag.append(static_cast<char>(0));
                        frame_tag.append(static_cast<quint8>(1));
                        frame_tag.append(static_cast<quint8>(2));
                        frame_tag.append(static_cast<quint8>(3));
                        frame_tag.append(static_cast<quint8>(4));
                        frame_tag.append(static_cast<quint8>(5));
                        frame_tag.append(static_cast<quint8>(6));
                        frame_tag.append(static_cast<quint8>(7));

                        footer = static_cast<quint8>(lineSplit.at(8).toUInt(nullptr, 10));
                        binning_X = static_cast<quint8>(lineSplit.at(9).toUInt(nullptr, 10));
                        binning_Y = static_cast<quint8>(lineSplit.at(10).toUInt(nullptr, 10));
                        nb_bits_sensor = static_cast<quint8>(lineSplit.at(11).toUInt(nullptr, 10));    // Shall be 10 or 12
                        test_mode = static_cast<quint8>(lineSplit.at(12).toUInt(nullptr, 10));
                        integration_time = static_cast<quint32>(lineSplit.at(13).toUInt(nullptr, 10));
                        binmode = static_cast<quint8>(lineSplit.at(14).toUInt(nullptr, 10));
                        nb_chan = static_cast<quint8>(lineSplit.at(15).toUInt(nullptr, 10));
                        ref_ADC_gain    = static_cast<quint8>(lineSplit.at(16).toUInt(nullptr, 10));
                        frame_idx_start = static_cast<quint8>(lineSplit.at(17).toUInt(nullptr, 10));
                        EOS_last        = static_cast<quint8>(lineSplit.at(18).toUInt(nullptr, 10));

                        compare = true;
                    }
                    else if (lineSplit.at(0).toUpper() == "RQST_VIDEO") {

                        bool pkt_ok = false;

                        qDebug().noquote() << "Requesting video ...";

                        quint8 nb_image = static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));
                        qDebug().noquote() << "Requesting video of " + QString::number(nb_image,10) +" images";
                        if (lineSplit.size() > 3) {
//                            this->spw_camera->setSPW_TimeoutVal(this->spw_camera->get_vid_channel(), lineSplit.at(4).toInt(nullptr, 10));
                            qDebug().noquote() << "Set CL timeout to " + QString::number(lineSplit.at(3).toInt(nullptr, 10),10);
                            this->Cl_camera->setCl_vid_TimeoutVal(lineSplit.at(3).toInt(nullptr, 10));
                        }

                        quint8 spw_nbbytes_per_pixel = 4; // static_cast<quint8>(lineSplit.at(3).toUInt(nullptr, 10));
                        quint16 ADC_gain;


                       ADC_gain = this->imx->Get_ADC();
                            //ADC_gain = 0x1A5;

                        // quint8 sens_freq_div;
                        double imx_freq = 74.25;
                        quint8 freqIndex = 0;
                        quint8 DDR_mode = 0;
                        quint8 adbit;
                        quint8 nb_bits_pxl;

                        quint8 sens_freq_0 =  static_cast<quint8>(this->Cl_camera->read_data(1, (AHBSLV_SPI_ba_CINEMA | IMX_CONF_INCKSEL0[cam_type])).at(0));
                        quint8 sens_freq_1 =  static_cast<quint8>(this->Cl_camera->read_data(1, (AHBSLV_SPI_ba_CINEMA | IMX_CONF_INCKSEL0[cam_type])).at(0));

                        if (sens_freq_1 == 2){
                          imx_freq = 37.125;
                          freqIndex= 2;}
                        else if (sens_freq_0 == 22){
                          imx_freq = 54;
                          freqIndex= 1;}
                         else{
                          imx_freq = 74.25;
                          freqIndex= 0;}

                        quint8 nb_chan_reg =  static_cast<quint8>(this->Cl_camera->read_data(1, (AHBSLV_SPI_ba_CINEMA | IMX_CONF_STBLVDS[cam_type])).at(0));
                        if (nb_chan_reg==0)
                          nb_chan = 16;
                        else if   (nb_chan_reg==1)
                          nb_chan = 8;
                        else
                          nb_chan = 4;

                         DDR_mode = static_cast<quint8>(this->Cl_camera->read_data(1, (AHBSLV_SPI_ba_CINEMA | IMX_CONF_FREQ[cam_type])).at(0));
                         adbit   =  static_cast<quint8>(this->Cl_camera->read_data(1, (AHBSLV_SPI_ba_CINEMA | IMX_CONF_ADBIT[cam_type])).at(0));
                         if (adbit == 0)
                             nb_bits_pxl = 10;
                         else if (adbit == 1)
                             nb_bits_pxl = 12;
                         else
                            nb_bits_pxl = 8;



                        // sens_freq_div = static_cast<quint8>(this->Cl_camera->read_data(1, SENS_CTL_ba | FREQ_DIV).at(0));

                        double  sens_in_freq;
                        quint8  Vramp1;
                        quint8  Vramp2;
                        quint16 Offset;
                        quint8  FOT;
                        double H_period;

                        sens_in_freq = imx_freq;
                        Vramp1       = 1;
                        Vramp2       = 1;
                        Offset       = 1;
                        FOT          = 1;
                        H_period     = this->imx->Get_H_period_IMX(freqIndex,nb_bits_pxl,nb_chan,DDR_mode);

                        /***************************/
//                        qDebug().noquote() << "IMX CONFIG INSIDE SCRIPT!";
//                        this->imx->config(nb_chan,    // int   nb_chan      , // Number of data channels
//                                              nb_bits_pxl, false, true);    // int   nb_bits       // 8, 10 or 12

//                        // copy of calib()
//                        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, (1 << 4));

//                        // Remove the SERDES reset
//                        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, 0x0);
                        /****************************/

                        //uptate CL buffer params

                        quint32 height    = quint32(this->Cl_camera->send_TM(AHBSLV_SPI_ba_CINEMA | IMX_ROI_1_WV_MSB[cam_type])<<8) + this->Cl_camera->send_TM(AHBSLV_SPI_ba_CINEMA | IMX_ROI_1_WV_LSB[cam_type]);

                        quint32 pixstart = quint32(this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_PIXSTART_AD1))<<8) + this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_PIXSTART_AD0));
                        quint32 pixlast  = quint32(this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_PIXLAST_AD1))<<8) + this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_PIXLAST_AD0));
                        quint32 width    = pixlast - pixstart + 1;

                        quint8 binX_val = this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_BINNING_X_AD));
                        quint8 binY_val = this->Cl_camera->send_TM((SENS_CTL_ba | CMVCTL_BINNING_Y_AD));

                        quint8 nb_frames_coadded = this->Cl_camera->send_TM((MC_DDC_ba))+1;
                        binX_val = static_cast<quint8>(pow(2, binX_val));
                        binY_val++;

                        this->Cl_camera->setCl_vid_TimeoutVal(10000);

//                        qDebug().noquote() << "nb_frames_coadded     = " + QString::number(nb_frames_coadded);
//                        qDebug().noquote() << "height    = " + QString::number(height);
//                        qDebug().noquote() << "width     = " + QString::number(width);
//                        qDebug().noquote() << "TapNumber = " + QString::number(this->TapNumber);

                        this->Cl_camera->Set_Cl_buffer_params(width, height, binX_val, binY_val, this->TapNumber);


                        quint8 nb_bits_final = nb_bits;

                        quint32 log2_bin_y = log2_int(static_cast<quint32>(binY_val));
                        quint32 log2_coadd = log2_int(static_cast<quint32>(nb_frames_coadded));

                        nb_bits_final = static_cast<quint8>(nb_bits_pxl + log2(binX_val) + log2_bin_y + log2_coadd);



                        this->saveThread->reset(0,imx_freq, static_cast<quint8>(ADC_gain),
                                                 1, 1,
                                                 static_cast<quint16>(Offset), nb_image, 1,
                                                 this->cam_fw_type, H_period);

                        this->save_BMP_Thread->reset(0, imx_freq, static_cast<quint8>(ADC_gain),
                                                     1, 1,
                                                     static_cast<quint16>(Offset), nb_image);

                        // release buffer
                        if (!this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning()){
                            for (quint8 i=0; i<NB_COPY_BUFFER; i++)
                                free(CopyBuffer[i]);
                        }


                        if (save_txt)
                            this->saveThread->start();
                        if (save_bmp)
                            this->save_BMP_Thread->start();
                        /*****************************/
                        this->Cl_camera->Grab();

                        Acquisition_params CL_Acq_params = Cl_camera->Get_CL_Acq_params();
                        for (quint8 i=0; i<NB_COPY_BUFFER; i++)
                             CopyBuffer[i] = (Int8*) malloc(CL_Acq_params.Buffsize * sizeof (Int8));
//                        qDebug().noquote() << "CL_Acq_params.Buffsize  " + QString::number(CL_Acq_params.Buffsize,10);

                        static uInt32 num_frame = 0;
                        static uInt32 actualCopiedBuffer = 0;


                        int MaxNbBuffer = int(NB_COPY_BUFFER-2.56*CL_Acq_params.Buffsize/1000000); //
                        if (MaxNbBuffer < 25)
                            MaxNbBuffer =  25;
                        if (MaxNbBuffer > NB_COPY_BUFFER)
                           MaxNbBuffer =  NB_COPY_BUFFER;
                        /*****************************/

                        //video request
                        //Sleep(100);

                        this->Cl_camera->send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, (1 << 4));
                        this->Cl_camera->send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, 0x0);
                        QByteArray data;
                        data.append(CMVCTL_CTL_VIDEO_REQ_gm);
                        this->Cl_camera->write_data(SENS_CTL_ba | CMVCTL_CTL_AD, data);


                        for (quint8 num_image = 0; num_image < nb_image; num_image++) {

                            line_read++;


                            int result = imgGetAttribute (CL_Acq_params.Sid, IMG_ATTR_LAST_VALID_FRAME, &num_frame);
                            num_frame++;

//                            qDebug().noquote() << " Acquisition  Frame :"  + QString::number(num_frame,10);



                            result = imgSessionCopyBufferByNumber(CL_Acq_params.Sid, num_frame, CopyBuffer[num_frame % MaxNbBuffer], IMG_OVERWRITE_GET_NEWEST, &actualCopiedBuffer, NULL);

//                            for (quint32 i = 0 ; i < 64 ; i++)
//                                qDebug().noquote() << "Frame "+ QString::number(num_frame) +" pixel " + QString::number(i) + ": 0x" + QString::number(CopyBuffer[i] & 0xff,16);

                          //  if (!result) {
//                                log_flux << "ERROR: Cannot Receive frame " << num_image << "\n";
//                                qDebug().noquote() <<  "ERROR: Cannot Receive frame " << num_image;
//                            }
//                            else {
                                log_flux << "Received frame " << num_image << "\n";
                                qDebug().noquote() << "Received frame " << num_image;

                                ref_height = height;

                                CU_Frame *frame = new CU_Frame(4, this->cam_fw_type);
                                frame->setNbBits(nb_bits_final);
                                frame->set_ref_height(ref_height);

//                                qDebug().noquote() << "nb bits final     = " + QString::number(nb_bits_final);
//                                qDebug().noquote() << "ref_height     = " + QString::number(ref_height);

                                //////////////////
                                pkt_ok = frame->init_pkt(CopyBuffer[num_frame % MaxNbBuffer]);

//                                qDebug().noquote()  << "  ID : " + QString::number(frame->getFrame_ID()) ;
//                                qDebug().noquote()  << "  pkt_ok : " + QString::number(pkt_ok) ;
//                                qDebug().noquote()  << "  save_txt : " + QString::number(save_txt) ;
                                if (pkt_ok && save_txt) {
//                                    qDebug().noquote()  << "  Creating image txt file.... " ;
//                                    QString txt_file_name = lineSplit.at(2)+ "_" + QString::number(num_image,10) + ".txt";
//                                    frame->create_txt_file(txt_file_name, sens_in_freq,
//                                                           ADC_gain, Vramp1, Vramp2,
//                                                           Offset, FOT, this->cam_fw_type, H_period);


//                                    log_flux << "  Frame saved to file " << txt_file_name << "\n";

                                 this->saveThread->add_cu_frame(frame, lineSplit.at(2), true);
                                }
//                                qDebug().noquote()  << "  save_bmp : " + QString::number(save_bmp) ;
                                if (pkt_ok && save_bmp) {
//                                    qDebug().noquote()  << "  Creating BMP file.... ";
//                                    QString bmp_file_name = lineSplit.at(2)+ "_" + QString::number(num_image,10) + ".bmp";
//                                    frame->toPixMap(this->SensorIsColor).save(bmp_file_name, "BMP");
//                                    log_flux << "  Frame saved to file " << bmp_file_name << "\n";
//                                    qDebug().noquote() << "  Frame saved to file " << bmp_file_name;
                                      this->save_BMP_Thread->add_pixmap(frame->toPixMap(this->SensorIsColor), lineSplit.at(2), true);
                                }

                                if (frame->isEOS() && num_image < nb_image-1) {
                                    log_flux << "ERROR: Number of received images is less than requested (Expected " << nb_image << ", received " << num_image+1 << ")\n";
                                    qDebug().noquote()  << "ERROR: Number of received images is less than requested (Expected " << nb_image << ", received " << num_image+1;
                                    break;
                                }
//                            }

                            this->progressBar->setValue(100*line_read/nb_line);
                        }

                        while (pkt_ok && (this->saveThread->isRunning() || this->save_BMP_Thread->isRunning()))
                        {
                            qDebug().noquote() << "Saving image files in progress...";
                            Sleep(5000);
                        }

                        // ReleaseCopyBuffer
//                        qDebug() << "Releasing Copy Buffers ...";
//                        for (quint8 i = 0; i<NB_COPY_BUFFER; i++)
//                            free(CopyBuffer[i]);
                    }
                    else if (lineSplit.at(0).toUpper() == "RCV_VIDEO") {

                        qDebug().noquote() << "Receiving video ...";

                        quint8 nb_image = static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));

                        if (lineSplit.size() > 4) {
//                            this->spw_camera->setSPW_TimeoutVal(this->spw_camera->get_vid_channel(), lineSplit.at(4).toInt(nullptr, 10));
                            this->Cl_camera->setCl_vid_TimeoutVal(lineSplit.at(4).toInt(nullptr, 10));
                        }

                        quint8 spw_nbbytes_per_pixel = 4; //static_cast<quint8>(lineSplit.at(3).toUInt(nullptr, 10));

                        QByteArray logicalAdd;
//                        logicalAdd.append(VID_LOGICAL_AD);

                        QPixArray picture_data;
//                        if (this->cam_fw_type != CAM_FW_CINEMA) {
//                            if (test_mode) {
//                                picture_data.clear();
//                                for (quint16 row_num = 0; row_num < 2048; row_num++) {
//                                    for (quint8 chan_num = 0; chan_num < nb_chan; chan_num++) {
//                                        for (quint16 pix_num = 0; pix_num < 2048/nb_chan; pix_num++) {
//                                            picture_data.append(pix_num);
//                                        }
//                                    }
//                                }
//                            }
//                        }
                        for (quint8 num_image = 0; num_image < nb_image; num_image++) {

                            line_read++;

                            bool status_ok;

                           /////////////// receive packet
//                         QByteArray video_pkt = this->spw_camera->SPW_rcv_pkt(&status_ok, this->spw_camera->get_vid_channel(), logicalAdd);
                            QByteArray video_pkt;
                            if (!status_ok) {
                                log_flux << "ERROR: Cannot Receive frame " << num_image << "\n";
                                qDebug().noquote() <<  "ERROR: Cannot Receive frame " << num_image;
                            }
                            else {
                                log_flux << "Received frame " << num_image << "\n";
                                qDebug().noquote() << "Received frame " << num_image;

                                CU_Frame *frame = new CU_Frame(spw_nbbytes_per_pixel, this->cam_fw_type);

                                frame->setNbBits(nb_bits);

                                frame->set_ref_height(ref_height);
//                                bool pkt_ok = frame->init_pkt(video_pkt);// CHANGER EN POiNTEUR
                                bool pkt_ok = true;
                                if (compare) {
                                    quint8 EOS = 0;
                                    if (num_image == nb_image-1) {
                                        EOS = EOS_last;
                                    }

                                    ref_frame = new CU_Frame(picture_data,
                                                             this->cam_fw_type,
                                                             nb_bits,
                                                             pix_start,
                                                             pix_last         ,
                                                             row_start        ,
                                                             nb_row           ,

                                                             black_lim ,
                                                             white_lim    ,

                                                             frame_idx_start + num_image,
                                                             nb_bytes    ,
                                                             frame_tag  ,
                                                             footer    ,
                                                             binning_X,
                                                             binning_Y,
                                                             binmode,
                                                             nb_bits_sensor,    // Shall be 10 or 12
                                                             test_mode,
                                                             integration_time,
                                                             ref_ADC_gain,
                                                             0,  // temperature
                                                             EOS);

                                    QVector<quint32> ignore_list;
                                    ignore_list.clear();
                                    ignore_list.append(34);  // Remove temperature Bytes from the list
                                    ignore_list.append(36);  // Remove temperature Bytes from the list
                                    if (footer) {
                                        for (quint32 i = 0; i < 8; i++) {
                                            ignore_list.append(ref_frame->getPacketSize()-1-i); // Remove CRC Bytes from the list
                                        }
                                    }
                                    if (!frame->compare(ref_frame, ignore_list)) {
                                        log_flux << "  ERROR when comparing received frame with reference frame\n";
                                        qDebug().noquote()  << "  ERROR when comparing received frame with reference frame\n";
                                    }
                                }

                                if (pkt_ok && save_txt) {

                                    QString txt_file_name = lineSplit.at(2)+ "_" + QString::number(num_image,10) + ".txt";

                                    frame->create_txt_file(txt_file_name, 6.4,
                                                           ref_ADC_gain, 0, 0,
                                                           0, 0, this->cam_fw_type, 0x444);
                                    log_flux << "  Frame saved to file " << txt_file_name << "\n";
                                    qDebug().noquote()  << "  Frame saved to file " << txt_file_name;
                                }

                                if (pkt_ok && save_bmp) {
                                    QString bmp_file_name = lineSplit.at(2)+ "_" + QString::number(num_image,10) + ".bmp";
                                    frame->toPixMap(this->SensorIsColor).save(bmp_file_name, "BMP");
                                    log_flux << "  Frame saved to file " << bmp_file_name << "\n";
                                    qDebug().noquote() << "  Frame saved to file " << bmp_file_name;
                                }

                                if ((frame->isEOS()) && (num_image < nb_image-1)) {
                                    log_flux << "ERROR: Number of received images is less than requested (Expected " << nb_image << ", received " << num_image+1 << ")\n";
                                    qDebug().noquote()  << "ERROR: Number of received images is less than requested (Expected " << nb_image << ", received " << num_image+1;
                                    break;
                                }

                            }

                            this->progressBar->setValue(100*line_read/nb_line);
                        }
                    }
                    else if (lineSplit.at(0).toUpper() == "CONF_NB_BITS") {
                        nb_bits = static_cast<quint8>(lineSplit.at(1).toUInt(nullptr, 10));
                    }
                    else if (lineSplit.at(0).toUpper() == "CONF_HEIGHT") {
                        ref_height = static_cast<quint16>(lineSplit.at(1).toUInt(nullptr, 10));
                    }
                    else if (lineSplit.at(0).toUpper() == "PRINT") {
                       QString print_msg = "  ";
                       for (int i = 1; i < lineSplit.size(); i++) {
                           print_msg.append(lineSplit.at(i));
                           print_msg.append(" ");
                       }
                       qDebug().noquote() << print_msg;
                       // log_flux << print_msg<< "\n";
                    }
                    else {
                        //TODO
                    }
                }
                else if (lineSplit.size() >= 1) {
                    if (lineSplit.at(0).toUpper() == "CLOSE_CL") {

                         this->Cl_camera->CameraLink_close();
                    }
                }
                this->progressBar->setValue(100*line_read/nb_line);
            }
            this->progressBar->setValue(100);
        }

        qDebug().noquote() << "End of script " << this->path;
        qDebug().noquote() << "";

        script_file.close();
        log_file.close();

        this->open_log->setVisible(true);

        this->start->setEnabled(true);
        this->close_cancel->setText("Close");
    }
    else {
        // Script File does NOT exist
        QString msg = "The file " + this->path + " does NOT exist";

        qDebug().noquote() << msg << "\n";

        if (! this->batch_mode) {
            QMessageBox msgBox;
            msgBox.setText(msg);
            msgBox.exec();
        }

        this->start->setEnabled(true);
        this->close_cancel->setText("Close");

    }
}


void script::on_browse_script_clicked(){

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Script 3D PLUS (*.3Dscript);; All files (*.*)"));
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList fileNames;
    if(dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        this->start->setDisabled(false);
        this->path = fileNames.at(0);
        this->file->setText("Script: " + fileNames.at(0));
        this->progressBar->setValue(0);
    }
    else
        dialog.reject();


}

void script::on_start_clicked()
{
    this->open_log->setVisible(false);

    this->start->setEnabled(false);
    this->close_cancel->setText("Cancel");

    this->run_script();
}

void script::on_close_cancel_clicked()
{
    if (this->close_cancel->text().toLower() == "close")
        this->close();
}

void script::on_open_log_clicked()
{
    QProcess *process = new QProcess();
    process->start("notepad " + this->path + ".log");
}


void script::EndThread(void)
{






//    qDebug().noquote() << "END Script";

}
