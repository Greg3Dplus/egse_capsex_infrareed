#include "mainwindow.h"


static Int8*        CopyBuffer=NULL;   // copied acquisition buffer


MainWindow::MainWindow(QWidget *parent, bool batch_mode) :
    QMainWindow(parent)
{

    this->init = new init_file();
    if (!this->init->fileExist())
    {
        QVariant list[17];
        QStringList listStr;
        listStr.append("sensor_params/test_mode");        list[0] = 0;
        listStr.append("sensor_params/binning_X");        list[1] = 1;
        listStr.append("sensor_params/binning_Y");        list[2] = 1;
        listStr.append("sensor_params/nb_bits");          list[3] = 10;
        listStr.append("sensor_params/f");          list[4] = 4;
        listStr.append("video_settings/nb_frames");       list[5] = 1;
        listStr.append("video_settings/width");           list[6] = 2048;
        listStr.append("video_settings/height");          list[7] = 2048;
        listStr.append("CameraLink_settings/tap_number"); list[8] = 0;
        listStr.append("Nand_flash_parameters/bypass");   list[9] = 0;
        listStr.append("BMP_settings/auto_save");         list[10] = 0;
        listStr.append("TXT_settings/auto_save");         list[11] = 4;
        listStr.append("Reserved/arg1");                  list[12] = 4;  // Camera HW type
        listStr.append("Reserved/arg2");                  list[13] = 3;  // Camera FW type
        this->init->set_value("init.ini", listStr, list);
    }

    this->init->get_value("Reserved", "arg1", 0, &this->cam_hw_type);
    this->init->get_value("Reserved", "arg2", 0, &this->cam_fw_type);
    this->init->get_value("sensor_params", "color", 0, &this->SensorIsColor);

    if (this->cam_fw_type == CAM_FW_CINEMA) {
        this->max_height = MAX_HEIGHT_IMX253;
        this->max_width  = MAX_WIDTH_IMX253;
    }else {
        this->max_height = MAX_HEIGHT_IMX990;
        this->max_width  = MAX_WIDTH_IMX990;
    }

    this->imx = new IMX(&this->gui_params.Cl_camera, AHBSLV_SPI_ba_CINEMA, this->cam_fw_type);
    this->cam_controller = new CAM_controller(this->imx, &this->gui_params.Cl_camera);

    this->xtrig_en = true;


    QStringList cam_name;
    cam_name.append("3DIPCC0746");
    cam_name.append("3DIPCC0802");
    cam_name.append("CO2M");
    cam_name.append("3DCM800");//CINEMA
    cam_name.append("CASPEX IR");
    qDebug().noquote() << "";
    qDebug().noquote()  << " 3D PLUS Micro Camera EGSE v" + QString::number(egse_version) + " (for " + cam_name.at(this->cam_fw_type) + ")";
    qDebug().noquote() << "";

    quint8 binning_X;
    this->init->get_value("sensor_params", "binning_X", 0, &binning_X);

    quint8 binning_Y;
    this->init->get_value("sensor_params", "binning_Y", 0, &binning_Y);

    this->init->get_value("sensor_params", "nb_bits", 10, &this->nb_bits_sensor);


    this->init->get_value("sensor_params", "nb_chan", 4, &this->nb_chan);

     this->init->get_value("Nand_flash_params", "bypass", 1, &this->fmc_bypass);

    this->init->get_value("CameraLink_settings", "tap_number", 1, &this->TapNumber);


//    this->init->get_value("SPW_settings", "spw_nbbytes_per_pixel",  3, &this->spw_nbbytes_per_pixel);
    spw_nbbytes_per_pixel = 4;
    this->spw_txdiv_cnt = 0;
    this->SysClk_freq = 128;
    this->SpwClk_freq = 200;

////////////////
    //this->gui_params.Cl_camera.set_CL_params(tapNumber);
//    this->gui_params.spw_camera.set_SPW_params(spw_port_num, spw_mult, spw_div);

    this->rcv_data_Byte = new CU_Frame(this->spw_nbbytes_per_pixel, this->cam_fw_type);

    this->strThread = new StreamingThreads(&this->gui_params,
                                           &this->rcv_data_Byte, &this->mutex);

    this->saveThread = new saveImageThread();
    this->save_BMP_Thread = new saveBMPThread();

    this->stream = false;
    this->pkt_received = false;
    this->num_sequence = 0;
    this->num_image_bmp = 0;
    this->configurate = false;

    if (!batch_mode)  { // Graphical User Interface
        this->fenetre = new QWidget();
        this->mainLayout   = new QGridLayout();
        this->statusBar = new QStatusBar();
        this->setStatusBar(this->statusBar);

        // MenuBar
        QPixmap progpix(":/res/img/program.png");
        QPixmap exitpix(":/res/img/exit.png");
        QPixmap aboutpix(":/res/img/about.png");
        QPixmap configpix(":/res/img/config.png");
        QPixmap exportpix(":/res/img/export.png");
        QPixmap openpix(":/res/img/open.png");
        QPixmap RWpix(":/res/img/read_write.png");

        this->actionProgram = new QAction(progpix, "&Program FPGA", this);
        this->actionProgram_SPI = new QAction(progpix, "&Program SPI NOR Flash", this);
        this->actionExit = new QAction(exitpix, "&Exit", this);
        this->actionRead_SENSOR_registers = new QAction(RWpix, "&Read/Write sensor's registers", this);
        this->actionConfigure_CL = new QAction(configpix, "&Configure CameraLink interface", this);
        this->actionSend_TC_TM = new QAction(RWpix, "&Read/Write any register of the camera", this);

        this->actionScript = new QAction(openpix, "&Launch script", this);
        this->action_exportScript = new QAction(exportpix,"&Export script to file", this);

        actionExit->setShortcut(tr("CTRL+Q"));
        actionRead_SENSOR_registers->setShortcut(tr("CTRL+I"));
        actionProgram->setShortcut(tr("CTRL+SHIFT+C"));

        this->actionAbout = new QAction(aboutpix, "&About", this);

        QObject::connect(actionProgram, SIGNAL(triggered()), this, SLOT(on_actionProgram_triggered()));
        QObject::connect(actionProgram_SPI, SIGNAL(triggered()), this, SLOT(on_actionProgram_SPI_triggered()));
        QObject::connect(actionExit,  SIGNAL(triggered()), this, SLOT(on_actionExit_triggered()));
        QObject::connect(this->actionRead_SENSOR_registers,  SIGNAL(triggered()), this, SLOT(on_actionRead_SENS_registers_triggered()));
        QObject::connect(actionConfigure_CL, SIGNAL(triggered()), this, SLOT(on_actionConfigure_CL_triggered()));
        QObject::connect(this->actionSend_TC_TM, SIGNAL(triggered()), this, SLOT(on_actionSend_TC_TM_triggered()));
        QObject::connect(this->actionScript, SIGNAL(triggered()), this, SLOT(on_actionScript_triggered()));
        QObject::connect(this->action_exportScript, SIGNAL(triggered()), this, SLOT(on_action_exportScript_triggered()));
        QObject::connect(actionAbout, SIGNAL(triggered()), this, SLOT(on_actionAbout_triggered()));

        QMenu *file;
        file = menuBar()->addMenu("&File");
        file->addAction(actionProgram);
        file->addAction(actionProgram_SPI);
        file->addSeparator();
        file->addAction(actionConfigure_CL);
        file->addSeparator();
        file->addAction(this->actionSend_TC_TM);
        file->addAction(this->actionRead_SENSOR_registers);
        file->addSeparator();
        file->addAction(this->actionScript);
        file->addAction(this->action_exportScript);
        file->addSeparator();
        file->addAction(actionExit);

        QMenu *about;
        about = menuBar()->addMenu("&About");
        about->addAction(actionAbout);

        this->first_column_widget = new QWidget();
        this->first_column = new QGridLayout();

        this->first_column_widget->setLayout(this->first_column);
        this->first_column_widget->setFixedWidth(245);

        this->mainLayout->addWidget(this->first_column_widget, 0, 0);

        this->cmv_params = new QGroupBox("Camera Unit Parameters");
        this->first_column->addWidget(this->cmv_params, 0, 0, 2, 2);

        this->cmv_params_layout = new QGridLayout();
        this->cmv_params->setLayout(this->cmv_params_layout);

        QLabel *ADC_lbl = new QLabel("ADC gain");

        ADC_lbl->setText("Gain (dB)");

        this->cmv_params_layout->addWidget(ADC_lbl, 0, 0);

        this->ADC_gain = new QSpinBox();
        this->cmv_params_layout->addWidget(this->ADC_gain, 0, 1);
        this->ADC_gain->setMinimum(0);

        this->ADC_gain->setMaximum(48);
        this->ADC_gain->setValue(24);

        QObject::connect(this->ADC_gain,     SIGNAL(valueChanged(int)),        this, SLOT(on_ADC_gain_valueChanged(int)));

        QLabel *exp_time_lbl = new QLabel("Exposure time");
        this->cmv_params_layout->addWidget(exp_time_lbl, 1, 0);

        this->exposure_doubleSpinBox = new QDoubleSpinBox();
        this->cmv_params_layout->addWidget(this->exposure_doubleSpinBox, 1, 1);
        this->exposure_doubleSpinBox->setMinimum(0);
        this->exposure_doubleSpinBox->setMaximum(10000);
        this->exposure_doubleSpinBox->setValue(1);
        this->exposure_doubleSpinBox->setSuffix(" ms");
        QObject::connect(this->exposure_doubleSpinBox,     SIGNAL(valueChanged(double)),        this, SLOT(on_exposure_doubleSpinBox_valueChanged(double)));

        QLabel *off_lbl = new QLabel("Offset");
        this->cmv_params_layout->addWidget(off_lbl, 2, 0);

        this->offset_val = new QSpinBox();
        this->cmv_params_layout->addWidget(this->offset_val, 2, 1);
        this->offset_val->setMinimum(0);
        this->offset_val->setMaximum(16383);

          this->offset_val->setMinimum(1);
          this->offset_val->setMaximum(4095);

          if (this->nb_bits_sensor==8)
              this->offset_val->setValue(15);
          else if (this->nb_bits_sensor==10)
              this->offset_val->setValue(60);
          else
              this->offset_val->setValue(240);


        QObject::connect(this->offset_val,     SIGNAL(valueChanged(int)),        this, SLOT(on_offset_val_valueChanged(int)));

        // Enable SPI NOR programming only for CO2M or CINEMA
        if (this->cam_hw_type != CAM_HW_CO2M && this->cam_hw_type != CAM_HW_CINEMA && this->cam_hw_type != CAM_HW_CASPEX_IR){
            actionProgram_SPI->setVisible(false);
        }

        this->binX_val = new CustomSpinBox();
//        if (this->cam_fw_type == CAM_FW_CO2M || this->cam_fw_type == CAM_FW_3DIPCC0802 || this->cam_fw_type == CAM_FW_CINEMA || this->cam_fw_type == CAM_FW_CASPEX_IR){
            QLabel *bin_x_lbl = new QLabel("Binning X");
            this->cmv_params_layout->addWidget(bin_x_lbl, 3, 0);

            this->cmv_params_layout->addWidget(this->binX_val, 3, 1);
//        }
        this->binX_val->setMinimum(1);
        this->binX_val->setMaximum(8);
        this->binX_val->setValue(binning_X);
        QObject::connect(this->binX_val,     SIGNAL(valueChanged(int)),        this, SLOT(on_binX_val_valueChanged()));

        this->binY_val = new QSpinBox();
//        if (this->cam_fw_type == CAM_FW_CO2M || this->cam_fw_type == CAM_FW_3DIPCC0802 || this->cam_fw_type == CAM_FW_CINEMA || this->cam_fw_type == CAM_FW_CASPEX_IR ){
            QLabel *bin_y_lbl = new QLabel("Binning Y");
            this->cmv_params_layout->addWidget(bin_y_lbl, 4, 0);

            this->cmv_params_layout->addWidget(this->binY_val, 4, 1);
//        }
        this->binY_val->setMinimum(1);
        this->binY_val->setMaximum(8);
        this->binY_val->setValue(binning_Y);
        QObject::connect(this->binY_val,     SIGNAL(valueChanged(int)),        this, SLOT(on_binY_val_valueChanged()));

        // Only for 3DIPCC0746 (Single QCheckbox to toggle binning)
        this->bin2x2 = new QCheckBox("Binning 2x2");
//        QObject::connect(this->bin2x2,     SIGNAL(stateChanged(int)),        this, SLOT(on_bin2x2_stateChanged()));

//        if (this->cam_fw_type != CAM_FW_CO2M && this->cam_fw_type != CAM_FW_3DIPCC0802 && this->cam_fw_type != CAM_FW_CINEMA){
//            this->bin2x2->setChecked(binning_X> 1 && binning_Y> 1);
//        }

        // Only for 3DIPCC0802
        this->bin_mode_val = new QComboBox();
        this->bin_mode_val->addItem("Sum");
        this->bin_mode_val->addItem("Average");
        QObject::connect(this->bin_mode_val,     SIGNAL(currentTextChanged(QString)),        this, SLOT(on_bin_mode_val_valueChanged()));

        this->test_mode = new QCheckBox("Test Mode");
        //if (this->cam_fw_type != CAM_FW_CINEMA)
            this->cmv_params_layout->addWidget(this->test_mode, 7, 0, 1, 2);
        QObject::connect(this->test_mode,     SIGNAL(stateChanged(int)),        this, SLOT(on_test_mode_stateChanged()));

        // rajouter bouton pour configurer le mode de test via une fenetre.


      this->test_mode_conf = new QPushButton("Test mode parameters");
      this->cmv_params_layout->addWidget(this->test_mode_conf, 7, 1, 1, 2);


       QObject::connect(this->test_mode_conf,     SIGNAL(clicked()),        this, SLOT(on_test_configuration_clicked())); //ouvrir fenetre
        //ONLY for CINEMA
        this->pix_8b = new QRadioButton(tr("&8 bits/pixel"));
        // Only for 3DIPCC0746 / 3DIPCC0802  / CINEMA
        this->pix_10b = new QRadioButton(tr("&10 bits/pixel"));
        this->pix_12b = new QRadioButton(tr("&12 bits/pixel"));


        QObject::connect(this->pix_8b,     SIGNAL(clicked()),        this, SLOT(on_pix_8b_clicked()));
        QObject::connect(this->pix_10b,    SIGNAL(clicked()),        this, SLOT(on_pix_10b_clicked()));
        QObject::connect(this->pix_12b,    SIGNAL(clicked()),        this, SLOT(on_pix_12b_clicked()));


        this->cmv_params_layout->addWidget(this->pix_10b, 9, 0);
        this->cmv_params_layout->addWidget(this->pix_12b, 10, 0);

        this->cmv_params_layout->addWidget(this->pix_8b, 8, 0);

        if (this->nb_bits_sensor == 12 ) {
            this->pix_12b->setChecked(true);
        }
        else if (this->nb_bits_sensor == 8) {
            this->pix_8b->setChecked(true);
        }
        else {
            this->pix_10b->setChecked(true);
        }

        this->rst_camera = new QPushButton("Reset sensor");
        this->cmv_params_layout->addWidget(this->rst_camera, 11, 0, 1, 2);
        QObject::connect(this->rst_camera,     SIGNAL(clicked()),        this, SLOT(on_rst_camera_clicked()));

        this->video_settings = new QGroupBox("Video settings");

        this->first_column->addWidget(this->video_settings, 5, 0, 2, 2);

        this->video_params_layout = new QGridLayout();
        this->video_settings->setLayout(this->video_params_layout);

        QLabel *nb_frames_lbl = new QLabel("Nb frames");
        this->video_params_layout->addWidget(nb_frames_lbl, 0, 0);

        this->nb_images_val = new QSpinBox();
        this->video_params_layout->addWidget(this->nb_images_val, 0, 1);
        this->nb_images_val->setMinimum(1);      
        this->nb_images_val->setMaximum(255);


        this->video_infinite = new QCheckBox("Infinite Video");
        this->video_params_layout->addWidget(this->video_infinite, 1, 1);
        QObject::connect(this->video_infinite,     SIGNAL(stateChanged(int)),        this, SLOT(on_video_infinite_stateChanged()));


        QLabel *nb_frames_coadded_lbl = new QLabel("Coadding frame nb");
        this->video_params_layout->addWidget(nb_frames_coadded_lbl, 2, 0);


        this->nb_frames_coadded = new QSpinBox();

        this->video_params_layout->addWidget(this->nb_frames_coadded, 2, 1);

        this->nb_frames_coadded->setMinimum(1);
        this->nb_frames_coadded->setMaximum(32);
        this->nb_frames_coadded->setValue(1);
        QObject::connect(this->nb_frames_coadded,     SIGNAL(valueChanged(int)),        this, SLOT(on_nb_frames_coadded_val_valueChanged(int)));

        quint8 nb_frames;
        this->init->get_value("video_settings", "nb_frames", 0, &nb_frames);
//        if (nb_frames == 255 && this->cam_fw_type != CAM_FW_CO2M && this->cam_fw_type != CAM_FW_CINEMA){
//            this->video_infinite->setChecked(true);
//        }
        if (nb_frames == 0 ){
            this->video_infinite->setChecked(true);
        }
        this->nb_images_val->setValue(nb_frames);
        QObject::connect(this->nb_images_val,     SIGNAL(valueChanged(int)),        this, SLOT(on_nb_images_val_valueChanged(int)));


        this->time_interval_val = new QDoubleSpinBox();
        this->fps_divider = new QSpinBox();
        if (this->xtrig_en  == true) {
            QLabel *frame_period_lbl = new QLabel("Frame period");
            this->video_params_layout->addWidget(frame_period_lbl, 3, 0);
            this->video_params_layout->addWidget(this->time_interval_val, 3, 1);
            this->time_interval_val->setMinimum(1);
            this->time_interval_val->setMaximum(10000);
            this->time_interval_val->setValue(200);
            this->time_interval_val->setSuffix(" ms");

        }
        else {
            QLabel *frame_period_lbl = new QLabel("Frame rate divider");

            this->video_params_layout->addWidget(frame_period_lbl, 3, 0);
            this->video_params_layout->addWidget(this->fps_divider, 3, 1);
            this->fps_divider->setMinimum(1);
            this->fps_divider->setMaximum(127);
            this->fps_divider->setValue(1);
       }
        this->fps_divider->setValue(1);

        QObject::connect(this->time_interval_val,     SIGNAL(valueChanged(double)),        this, SLOT(on_time_interval_val_valueChanged(double)));
        QObject::connect(this->fps_divider, SIGNAL(valueChanged(int)), this, SLOT(on_fps_divider_valueChanged(int)));

        this->FPS = new QLabel();
        this->video_params_layout->addWidget(this->FPS, 4, 0, 1, 2);

        this->max_FPS = new QLabel();
        this->video_params_layout->addWidget(this->max_FPS, 5, 0, 1, 2);

        this->window_dim = new QGroupBox("Window Parameters");
//        if (this->cam_hw_type != CAM_HW_CINEMA)
        this->first_column->addWidget(this->window_dim, 3, 0, 1, 2);
//        else {
//            this->first_column->addWidget(this->window_dim, 2, 0, 1, 2);
//        }
        this->window_params_layout = new QGridLayout();
        this->window_dim->setLayout(this->window_params_layout);

        QLabel *width_lbl = new QLabel("Width");
        this->window_params_layout->addWidget(width_lbl, 0, 0);

        this->width = new QSpinBox();
        this->window_params_layout->addWidget(this->width, 0, 1);
        this->width->setMinimum(1);
        this->width->setMaximum(this->max_width);
        this->width->setValue(this->max_width);

        QLabel *height_lbl = new QLabel("Height");
        this->window_params_layout->addWidget(height_lbl, 1, 0);

        this->height = new QSpinBox();
        this->window_params_layout->addWidget(this->height, 1, 1);
        this->height->setMinimum(4);
        this->height->setSingleStep(4);
        this->height->setMaximum(this->max_height);
        this->height->setValue(this->max_height);
        QObject::connect(this->height,     SIGNAL(valueChanged(int)),        this, SLOT(on_height_valueChanged(int)));

        QLabel *off_x_lbl = new QLabel("Offset X");
        this->window_params_layout->addWidget(off_x_lbl, 2, 0);

        this->offset_x = new QSpinBox();
        this->window_params_layout->addWidget(this->offset_x, 2, 1);
        this->offset_x->setMinimum(0);
        this->offset_x->setMaximum(this->max_width-8);
        this->offset_x->setValue(0);
        QObject::connect(this->offset_x,     SIGNAL(valueChanged(int)),        this, SLOT(on_offset_x_valueChanged(int)));

        QLabel *off_y_lbl = new QLabel("Offset Y");
        this->window_params_layout->addWidget(off_y_lbl, 3, 0);

        this->offset_y = new QSpinBox();
        this->window_params_layout->addWidget(this->offset_y, 3, 1);
        this->offset_y->setMinimum(0);
        this->offset_y->setMaximum(this->max_height-1);
        this->offset_y->setValue(0);
        QObject::connect(this->offset_y,     SIGNAL(valueChanged(int)),        this, SLOT(on_offset_y_valueChanged(int)));

        this->freq_params = new QGroupBox("Frequency Parameters"); 
        this->first_column->addWidget(this->freq_params, 2, 0, 1, 2);

        this->freq_params_layout = new QGridLayout();
        this->freq_params->setLayout(this->freq_params_layout);
        quint8 freq_params_Y = 0;



//        if (this->cam_fw_type == CAM_FW_CINEMA ) {
            QLabel *cmv_freq_lbl = new QLabel("IMX Freq (MHz)");
            this->freq_params_layout->addWidget(cmv_freq_lbl, freq_params_Y, 0);

            this->imx_freq_div = new QComboBox();
            this->freq_params_layout->addWidget(this->imx_freq_div, freq_params_Y++, 1, 2, 1);
            this->imx_freq_div->addItem("74.25");
            this->imx_freq_div->addItem("54");
            this->imx_freq_div->addItem("37.125");
            QObject::connect(this->imx_freq_div,     SIGNAL(currentTextChanged(QString)),        this, SLOT(on_IMX_freq_div_valueChanged()));
//        }
//        else
//        {
//            QLabel *cmv_freq_lbl = new QLabel("CMV Freq Div");
//            this->freq_params_layout->addWidget(cmv_freq_lbl, freq_params_Y, 0, 2 ,1);

//            this->CMV_freq_div = new QSpinBox();
//            this->freq_params_layout->addWidget(this->CMV_freq_div, freq_params_Y++, 1, 2, 1);
//            this->CMV_freq_div->setMinimum(1);
//            this->CMV_freq_div->setMaximum(18);
//            this->CMV_freq_div->setValue(1);
//            if (this->cam_fw_type == CAM_FW_3DIPCC0746 || this->cam_fw_type == CAM_FW_3DIPCC0802){ // Frequency shall be between 5 and 9 MHz
//                this->CMV_freq_div->setMinimum(7);
//                this->CMV_freq_div->setMaximum(11);
//                this->CMV_freq_div->setValue(9); // 6.4MHz by default
//            }
//            QObject::connect(this->CMV_freq_div,     SIGNAL(valueChanged(int)),        this, SLOT(on_CMV_freq_div_valueChanged(int)));
//        }


                this->DDR_mode_disable = new QCheckBox("Sensor DDR mode disable");
                this->freq_params_layout->addWidget(DDR_mode_disable, freq_params_Y++, 0, 2, 1);
                QObject::connect(this->DDR_mode_disable,     SIGNAL(stateChanged(int)),        this, SLOT(on_DDR_mode_stateChanged()));

                QLabel *nb_chan_lbl = new QLabel("Number of channels");
                this->freq_params_layout->addWidget(nb_chan_lbl, freq_params_Y, 0, 2, 1);

                this->imx_nb_chan = new QComboBox();
                this->freq_params_layout->addWidget(this->imx_nb_chan, freq_params_Y++, 1, 2, 1);
                this->imx_nb_chan->addItem("4");
                this->imx_nb_chan->addItem("8");
                this->imx_nb_chan->addItem("16");
                if (this->nb_chan == 4) {
                    this->imx_nb_chan->setCurrentIndex(0);
                }
                else if (this->nb_chan == 8) {
                   this->imx_nb_chan->setCurrentIndex(1);
                }
                else {
                   this->imx_nb_chan->setCurrentIndex(2);
                }

                QObject::connect(this->imx_nb_chan,     SIGNAL(currentTextChanged(QString)),        this, SLOT(on_IMX_nb_chan_valueChanged()));

                // Disabled for preliminary version
                //if (PRELIMINARY_VER == true) {
                  this->DDR_mode_disable->setDisabled(true);
                  // this->pix_10b->setDisabled(true);
                  // this->pix_12b->setDisabled(true);
                 // this->binX_val->setMaximum(2);
                  //this->binY_val->setMaximum(8);
                  this->imx_freq_div->setDisabled(true);
                 // this->imx_nb_chan->setDisabled(true);
                //}


        this->cmv_in_freq_lbl = new QLabel();
        this->freq_params_layout->addWidget(this->cmv_in_freq_lbl, freq_params_Y++, 0, 1, 2);

        this->pixel_rate_lbl = new QLabel();
        this->freq_params_layout->addWidget(this->pixel_rate_lbl, freq_params_Y++, 0, 2, 2);

        this->Nand_flash_group = new QGroupBox("Nand Flash Parameters");
        this->first_column->addWidget(this->Nand_flash_group, 4, 0, 1, 2);

        this->Nand_flash_layout = new QGridLayout();
        this->Nand_flash_group->setLayout(this->Nand_flash_layout);


        this->Nand_Bypass = new QCheckBox("Bypass Mode");
//        if (this->cam_fw_type == CAM_FW_CINEMA)
            this->Nand_flash_layout->addWidget(this->Nand_Bypass, 1, 0);

        QObject::connect(this->Nand_Bypass,     SIGNAL(stateChanged(int)),        this, SLOT(on_NAND_bypass()));

        this->read_last_button = new QPushButton("Read last sequence");
//        if (this->cam_fw_type == CAM_FW_CINEMA)
            this->Nand_flash_layout->addWidget(this->read_last_button, 2, 0);
        this->read_last_button->setEnabled(false);
        QObject::connect(this->read_last_button,     SIGNAL(clicked()),        this, SLOT(on_Read_last_sequence_clicked()));

        this->Nand_Bypass->setChecked(this->fmc_bypass);


        this->ConfigButton = new QPushButton("Config");
        QPixmap confpix(":/res/img/config_button.png");
        this->ConfigButton->setIcon(QIcon(confpix));
        this->ConfigButton->setIconSize(QSize(16, 16));
        this->first_column->addWidget(this->ConfigButton, 7, 0);
        QObject::connect(this->ConfigButton,     SIGNAL(clicked()),        this, SLOT(on_ConfigButton_clicked()));

        this->SPW_running = new QPushButton();
        this->first_column->addWidget(this->SPW_running, 7, 1);
        this->SPW_running->setDisabled(true);
        this->SPW_running->setVisible(false);

        this->StartButton = new QPushButton("Start");
        QPixmap startpix(":/res/img/start_button.png");
        this->StartButton->setIcon(QIcon(startpix));
        this->StartButton->setIconSize(QSize(16, 16));

        this->first_column->addWidget(this->StartButton, 8, 0);
        QObject::connect(this->StartButton,     SIGNAL(clicked()),        this, SLOT(on_StartButton_clicked()));

        this->Error = new QPushButton();
        this->first_column->addWidget(this->Error, 8, 1);
        this->Error->setDisabled(true);
        this->Error->setVisible(false);

        this->AbortButton = new QPushButton("Abort");
        QPixmap abortpix(":/res/img/abort_button.png");
        this->AbortButton->setIcon(QIcon(abortpix));
        this->AbortButton->setIconSize(QSize(16, 16));
        this->first_column->addWidget(this->AbortButton, 9, 0);
        QObject::connect(this->AbortButton,     SIGNAL(clicked()),        this, SLOT(on_AbortButton_clicked()));


//        this->CloseButton = new QPushButton("Close Cl");
//        this->CloseButton->setIcon(QIcon(abortpix));
//        this->CloseButton->setIconSize(QSize(16, 16));
//        this->first_column->addWidget(this->CloseButton, 8, 0);
//        QObject::connect(this->CloseButton,     SIGNAL(clicked()),        this, SLOT(on_CloseButton_clicked()));

        this->screen_lbl = new QLabel();

        scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(screen_lbl);
        screen_lbl->setAlignment(Qt::AlignCenter);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setMinimumWidth(250);
        this->mainLayout->addWidget(scrollArea, 0, 1);

        // Last Column
        this->last_column_widget = new QWidget();
        this->last_column = new QGridLayout();
        this->last_column_widget->setLayout(this->last_column);
        this->last_column_widget->setFixedWidth(250);

        this->mainLayout->addWidget(this->last_column_widget, 0, 2);

        this->real_time_params = new QGroupBox("Real Time Information");
        this->last_column->addWidget(this->real_time_params, 0, 0, 1, 2);
        this->real_time_params_layout = new QGridLayout();
        this->real_time_params->setLayout(this->real_time_params_layout);

        this->temperature_label = new QLabel("No video has been started yet");
        this->real_time_params_layout->addWidget(this->temperature_label, 0, 0);

        // Last column, 2nd group box
        this->status_params = new QGroupBox("Status");
        this->last_column->addWidget(this->status_params, 1, 0, 1, 2);
        this->status_params_layout = new QGridLayout();
        this->status_params->setLayout(this->status_params_layout);

        this->test_label = new QLabel("No video has been started yet");
        this->status_params_layout->addWidget(this->test_label, 0, 0);

        this->file_status = new QLabel("No video has been started yet");
        this->status_params_layout->addWidget(this->file_status, 1, 0);

        this->valid_hardware = new QLabel();
        this->valid_hardware->setWordWrap(true);
        this->status_params_layout->addWidget(this->valid_hardware, 2, 0);

        //
        this->save_TXT_pb = new QPushButton("Save to TXT file");
        QPixmap txtpix(":/res/img/txt_button.png");
        this->save_TXT_pb->setIcon(QIcon(txtpix));
        this->save_TXT_pb->setIconSize(QSize(16, 16));
        this->last_column->addWidget(this->save_TXT_pb, 4, 0);
        QObject::connect(this->save_TXT_pb,     SIGNAL(clicked()),        this, SLOT(on_save_TXT_pb_clicked()));

        this->save_all_TXT = new QCheckBox("Save ALL pictures");
        this->last_column->addWidget(this->save_all_TXT, 4, 1);

        this->save_BMP_pb = new QPushButton("Save to BMP file");
        QPixmap bmppix(":/res/img/bmp_button.png");
        this->save_BMP_pb->setIcon(QIcon(bmppix));
        this->save_BMP_pb->setIconSize(QSize(16, 16));
        this->last_column->addWidget(this->save_BMP_pb, 5, 0);
        QObject::connect(this->save_BMP_pb,     SIGNAL(clicked()),        this, SLOT(on_save_BMP_pb_clicked()));

        this->save_all_BMP = new QCheckBox("Save ALL pictures");
        this->last_column->addWidget(this->save_all_BMP, 5, 1);

        QPixmap Logo(":/res/img/logo.png");
        QLabel* logo_lbl = new QLabel();
        logo_lbl->setMaximumWidth(100);
        logo_lbl->setPixmap(Logo.scaled(logo_lbl->width(), logo_lbl->height(), Qt::KeepAspectRatio));
        logo_lbl->setAlignment(Qt::AlignCenter);
        this->last_column->addWidget(logo_lbl, 6, 0, 1, 2);

        this->fenetre->setLayout(this->mainLayout);
        this->fenetre->adjustSize();
        setCentralWidget(this->fenetre);

        this->setWindowTitle("3D PLUS Micro Camera EGSE v" + QString::number(egse_version) + " (for " + cam_name.at(this->cam_fw_type) + ")");
        //setWindowIcon(QIcon("camera.PNG"));

        quint8 auto_save_bmp;
        this->init->get_value("BMP_settings", "auto_save", 0, &auto_save_bmp);
        this->save_all_BMP->setChecked(auto_save_bmp);

        QObject::connect(this->save_all_BMP,     SIGNAL(stateChanged(int)),        this, SLOT(on_autosave_isChecked()));

        quint8 auto_save_txt;
        this->init->get_value("TXT_settings", "auto_save", 0, &auto_save_txt);
        this->save_all_TXT->setChecked(auto_save_txt);
        QObject::connect(this->save_all_TXT,     SIGNAL(stateChanged(int)),        this, SLOT(on_autosave_isChecked()));

        quint8 test_mode;
        this->init->get_value("sensor_params", "test_mode", 0, &test_mode);

        quint16 width;
        this->init->get_value("video_settings", "width", 0, &width);
        this->width->setValue(width);

        quint16 height;
        this->init->get_value("video_settings", "height", 0, &height);
        this->height->setValue(height);

        this->move_to_center = true;

        this->AbortButton->setDisabled(true);
        this->StartButton->setEnabled(false);

        this->save_BMP_pb->setDisabled(true);
        this->save_TXT_pb->setDisabled(true);

        this->on_binX_val_valueChanged();
        this->on_binY_val_valueChanged();

        this->compute_FPS();

        this->Error->setVisible(false);
        this->SPW_running->setVisible(false);

        this->window_send_TMTC = new sendcmddialog();
        connect(this->window_send_TMTC,SIGNAL(TM_clicked()),this,SLOT(show_TM()));
        connect(this->window_send_TMTC,SIGNAL(TC_clicked()),this,SLOT(transmit_TC()));

        this->actionRead_SENSOR_registers->setEnabled(false);
        this->test_mode_conf->setEnabled(true);
        this->actionSend_TC_TM->setEnabled(false);

        this->LabelStatusBar = new QLabel("Ready");
        this->statusBar->addPermanentWidget(this->LabelStatusBar);

        QObject::connect(this->width,     SIGNAL(valueChanged(int)),        this, SLOT(on_width_valueChanged(int)));

        connect(this->strThread, &StreamingThreads::FrameReceived, this, &MainWindow::streaming);


         //a refaire pour CL
//        connect(&(this->gui_params.spw_camera),     &SPW_camera::PACKET_SIZE_ERROR  , this, &MainWindow::SetError);

       connect(this->strThread,                    &StreamingThreads::END_THREAD   , this, &MainWindow::EndThread);
       connect(this->strThread,                    &StreamingThreads::END_SEQUENCE , this, &MainWindow::EndSequence);

        //a refaire pour CL
//        connect(&(this->gui_params.spw_camera),     &SPW_camera::CHANNEL_NOT_OPEN   , this, &MainWindow::ChannelNotOpen);

        connect(this->rcv_data_Byte,                &CU_Frame::CU_error             , this, &MainWindow::SetError);

        connect(this->saveThread,     &saveImageThread::END_THREAD , this, &MainWindow::EndThread);
        connect(this->saveThread,     SIGNAL(NEW_SAVE(QString))    , this, SLOT(PrintStatusBar(QString)));

        connect(this->save_BMP_Thread,  &saveBMPThread::END_THREAD , this, &MainWindow::EndThread);
        connect(this->save_BMP_Thread,  SIGNAL(NEW_SAVE(QString))    , this, SLOT(PrintStatusBar(QString)));


        Enable_IHM_param(false);
    }



}

MainWindow::~MainWindow()
{

}

/*******************************************************/

void MainWindow::run_script(QString script_name, bool batch_mode) {
    scriptWindow = new script(this->cam_controller,this->imx, &this->gui_params.Cl_camera, script_name, this->cam_fw_type,this->SensorIsColor, batch_mode);
    scriptWindow->run_script();
    scriptWindow->close();
}

void  MainWindow::Abort_Video(void) {
    //this->gui_params.spw_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, CMVCTL_CTL_ABORT_REQ_gm);
    this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, CMVCTL_CTL_ABORT_REQ_gm);



}

void MainWindow::SendVidReq(void)
{

    this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, (1 << 4));
    //this->imx->enable_standby_mode(true);
    // Remove the SERDES reset
    this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, 0x0);
    this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, CMVCTL_CTL_VIDEO_REQ_gm);

}

void MainWindow::SendReadLast(void)
{

    this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, MCFMC_READ_CONFIRM_MODE);
    this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, MCFMC_READ_LAST_REQ);
    this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, MCFMC_READ_NOMINAL_MODE);

}

void MainWindow::on_NAND_bypass(void)
{
     if (this->Nand_Bypass->isChecked()) {

         this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, MCFMC_BYPASS_MODE);

     }
     else
     {
         this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, MCFMC_READ_NOMINAL_MODE);
     }


     if (this->Nand_Bypass->isChecked()==false && this->last_seq_enable == true)
         this->read_last_button->setEnabled(true); // true
     else
         this->read_last_button->setEnabled(false);

}

bool MainWindow::calib(void) {

        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, (1 << 4));

        // Remove the SERDES reset
        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, 0x0);

        // qDebug().noquote() << "////// ATTENTION //////////";
        // this->gui_params.spw_camera.send_TC(0x10438, 0x0);

        // qDebug().noquote() << "////// ATTENTION BLACK LEVEL AUTO ADJUST REMOVED//////////";
        // this->gui_params.spw_camera.send_TC(0x10222, 0xF0);

        // Sleep(100);
        // this->imx->enable_standby_mode(false);
        // Sleep(100);

        // Set training word to 0x1
//        this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | SENSCTL_TRAINING_PATTERN_AD0, 0x0001);

//        Sleep(10);
        // Request Calibration
//        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, CMVCTL_CTL_TRAINING_gm);

//        Sleep(100);

        // Check that training is done
//        quint8 SENS_STS = this->gui_params.Cl_camera.send_TM(SENS_CTL_ba | CMVCTL_STS_AD); // send_TM(nb_data, addr)

//        if ((SENS_STS & CMVCTL_STS_TRDONE_gm) == 0) {
//            qDebug () << "ERROR: Training is NOT done";
//            qDebug () <<"SENS_STS = 0x" + QString::number(SENS_STS, 16);
//            return 0;
//        }

    //Read calibration status for each channel
//    QByteArray Calib_sts = this->gui_params.Cl_camera.send_TM(2,SENS_CTL_ba | CALIB_STS_AD0);
//    if (this->nb_chan == 16)
//        if (Calib_sts.at(0)== 0x02)
//            qDebug().noquote() << "Calibration status ok";
//        else {
//            qDebug().noquote() << "Calibration fail";
//            qDebug().noquote() << "SENS_Calib_sts = 0x" + QString::number(Calib_sts.at(0),16);   }
//    else {
//        if (Calib_sts.at(0)== 0x01)
//            qDebug().noquote() << "Calibration status ok";
//        else {
//            qDebug().noquote() << "Calibration fail";
//            qDebug().noquote() << "SENS_Calib_sts = 0x" + QString::number(Calib_sts.at(0),16);   }
//    }

    return 1;
}


// bin_mode: 0: Sum  1: Average
quint8 MainWindow::config_Binning(quint8 binning_X, quint8 binning_Y, quint8 bin_mode)
{
//    if (this->cam_fw_type == CAM_FW_CO2M || this->cam_fw_type == CAM_FW_CINEMA){
        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_BINNING_X_AD, static_cast<quint8>(log2_int(binning_X)));
        this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | CMVCTL_BINNING_Y_AD, binning_Y-1);
//    }
//    else if (this->cam_fw_type == CAM_FW_3DIPCC0802){
//        this->gui_params.spw_camera.send_TC(SENS_CTL_ba | CMVCTL_BINNING_X_AD, static_cast<quint8>((bin_mode                                 << 2) |
//                                                                                                  (static_cast<quint8>(log2_int(binning_X)) << 0)));
//        this->gui_params.spw_camera.send_TC(SENS_CTL_ba | CMVCTL_BINNING_Y_AD, binning_Y-1);
//    }
//    else {
//        // Modify only the required bits
//        quint8 cmv_conf_val = this->gui_params.spw_camera.send_TM(SENS_CTL_ba | CMVCTL_CONF_AD);

//        if (log2(static_cast<double>(binning_X))==1 && (binning_Y-1) == 1) {
//            cmv_conf_val = cmv_conf_val | 1;
//        }
//        else {
//            cmv_conf_val = cmv_conf_val & ~ 1;
//        }

//        this->gui_params.spw_camera.send_TC(SENS_CTL_ba | CMVCTL_CONF_AD, cmv_conf_val);
//    }

    if (this->sensor == 0)
        return 0;

    return 1;
}

void MainWindow::on_StartButton_clicked()
{

    if (!this->Nand_Bypass->isChecked())
        this->last_seq_enable = true; // enable last sequence read

    this->on_ConfigButton_clicked();

    this->LabelStatusBar->setText("Running...");

    // Update of exposure_time during RUN is forbidden
    this->exposure_doubleSpinBox->setDisabled(true);

    this->offset_val->setDisabled(true);
    this->ADC_gain->setDisabled(true);
    this->binX_val->setDisabled(true);
    this->binY_val->setDisabled(true);
    this->test_mode->setDisabled(true);
    this->Nand_Bypass->setDisabled(true);
    this->read_last_button->setDisabled(true);
    this->window_dim->setDisabled(true);
    this->StartButton->setDisabled(true);
    this->video_settings->setDisabled(true);
    this->AbortButton->setEnabled(true);

    this->imx_freq_div->setDisabled(true);

    this->ConfigButton->setDisabled(true);
    this->save_all_BMP->setDisabled(true);
    this->save_all_TXT->setDisabled(true);

    this->save_BMP_pb->setEnabled(true);
    this->save_TXT_pb->setEnabled(true);

    this->rst_camera->setDisabled(true);

    this->bin2x2->setDisabled(true);
    this->imx_nb_chan->setDisabled(true);

    this->pix_8b->setDisabled(true);

    this->pix_10b->setDisabled(true);
    this->pix_12b->setDisabled(true);

    this->actionProgram->setDisabled(true);
    this->actionProgram_SPI->setDisabled(true);
    this->actionConfigure_CL->setDisabled(true);
    this->actionScript->setDisabled(true);
    this->actionRead_SENSOR_registers->setDisabled(true);//true
    this->test_mode_conf->setDisabled(true);//true

    quint32 log2_bin_y = log2_int(static_cast<quint32>(this->binY_val->value()));
    quint32 log2_coadd = log2_int(static_cast<quint32>(this->nb_frames_coadded->value()));

    quint8 nb_bits_final = this->nb_bits_sensor;

//    if (this->cam_fw_type == CAM_FW_CO2M){
//        nb_bits_final = static_cast<quint8>(10 + log2(this->binX_val->value()) + log2_bin_y + log2_coadd);
//    }
//    else if (this->cam_fw_type == CAM_FW_CINEMA) {
        nb_bits_final = static_cast<quint8>(this->nb_bits_sensor + log2(this->binX_val->value()) + log2_bin_y + log2_coadd);
//    }
//    else if (this->cam_fw_type == CAM_FW_3DIPCC0802 && this->bin_mode_val->currentIndex() == 0) {
//        nb_bits_final = static_cast<quint8>(this->nb_bits_sensor + log2(this->binX_val->value()) + log2_bin_y);
//    }

    if (nb_bits_final > 8*this->spw_nbbytes_per_pixel) {
        nb_bits_final = 8*this->spw_nbbytes_per_pixel;
    }

    this->rcv_data_Byte->setCamType(this->cam_fw_type);
    this->rcv_data_Byte->setNbBits(nb_bits_final);
    this->rcv_data_Byte->set_ref_height(static_cast<quint16>(this->height->value() / this->binY_val->value()));

    quint8 mult_X = 8;

    if ((this->offset_x->value() % (mult_X*this->binX_val->value())) != 0  && (this->offset_x->value() % (this->nb_chan)) != 0) {
        quint16 new_offset_x = static_cast<quint16>(this->offset_x->value() - (this->offset_x->value() % (this->nb_chan)*mult_X*this->binX_val->value()));

        QMessageBox msgBox;
        QString msg = "Invalid offset X: " + QString::number(this->offset_x->value()) + " replaced by " + QString::number(new_offset_x);
        msgBox.setText(msg);
        msgBox.exec();
        this->offset_x->setValue(new_offset_x);
    }
    quint8 mult_Y = 4;
    if ((this->offset_y->value() % (mult_X*this->binX_val->value())) != 0) {
        quint16 new_offset_y = static_cast<quint16>(this->offset_y->value() - (this->offset_y->value() % (mult_X*this->binX_val->value())));

        QMessageBox msgBox;
        QString msg = "Invalid offset Y: " + QString::number(this->offset_y->value()) + " replaced by " + QString::number(new_offset_y);
        msgBox.setText(msg);
        msgBox.exec();
        this->offset_y->setValue(new_offset_y);
    }

        if ((this->width->value() % (mult_X*this->binX_val->value())) != 0 && (this->width->value() % (this->nb_chan)) != 0) {
            quint16 new_width = static_cast<quint16>(this->width->value() - (this->width->value() % (this->nb_chan*mult_X*this->binX_val->value())));
            if (new_width == 0) {
                new_width = static_cast<quint16>(this->width->minimum());
            }

            QMessageBox msgBox;
            QString msg = "Invalid width:" + QString::number(this->width->value()) + " replaced by " + QString::number(new_width);
            msgBox.setText(msg);
            msgBox.exec();
            this->width->setValue(new_width);
        }

//    if (this->cam_hw_type != CAM_HW_CINEMA){
//        if (this->nb_images_val->value() > 1 && this->exposure_doubleSpinBox->value() >= this->time_interval_val->value()) {
//            double new_exp = this->exposure_doubleSpinBox->value() - 1;
//            QMessageBox msgBox;
//            QString msg = "Invalid exposure time:" + QString::number(this->exposure_doubleSpinBox->value()) + " ms replaced by " + QString::number(new_exp) + " ms";
//            msgBox.setText(msg);
//            msgBox.exec();
//            this->exposure_doubleSpinBox->setValue(new_exp);
//        }
//    }
    if ((this->width->value() + this->offset_x->value()) > this->max_width) {
        this->width->setValue(this->max_width - this->offset_x->value());

        QMessageBox msgBox;
        QString msg = "Too many pixels in the line. Width replaced by " + QString::number(this->width->value());
        msgBox.setText(msg);
        msgBox.exec();
    }

    if (this->height->value()%(mult_Y*this->binY_val->value()) != 0){
       this->height->setValue(this->height->value() - this->height->value() % (mult_Y*this->binY_val->value()));
        QMessageBox msgBox;
        QString msg = "Invalid height (not a multiple of 4 x Binning Y) . Height replaced by " + QString::number(this->height->value());
        msgBox.setText(msg);
        msgBox.exec();

    }

    if ((this->height->value() + this->offset_y->value()) > this->max_height) {

        this->height->setValue(this->max_height - this->offset_y->value());

        QMessageBox msgBox;
        QString msg = "Too many lines in the frame. Height replaced by " + QString::number(this->height->value());
        msgBox.setText(msg);
        msgBox.exec();
    }


    if ((this->nb_images_val->value() % this->nb_frames_coadded->value()) != 0) {

        this->nb_images_val->setValue(this->nb_images_val->value() + this->nb_frames_coadded->value() - (this->nb_images_val->value() % this->nb_frames_coadded->value()));

        QMessageBox msgBox;
        QString msg = "The number of frames shall be a multiple of the coadding parameter. Replaced by " + QString::number(this->nb_images_val->value());
        msgBox.setText(msg);
        msgBox.exec();
    }

    int timeout_ms;

    timeout_ms = (int(this->time_interval_val->value())+100)*(nb_images_val->value()+8) + 5000*!this->Nand_Bypass->isChecked()*nb_images_val->value();
//    timeout_ms = 25000;
//    qDebug().noquote() << "Timeout (ms): " + QString::number(timeout_ms, 10);


//    this->gui_params.spw_camera.setSPW_vid_TimeoutVal(timeout_ms);
    this->gui_params.Cl_camera.setCl_vid_TimeoutVal(timeout_ms);

    this->mutex.lock();
    quint8 nb_output_images = static_cast<quint8>(this->nb_images_val->value() / this->nb_frames_coadded->value());

   this->saveThread->reset(this->num_sequence,this->cam_controller->Get_IMX_Freq(this->imx_freq_div->currentIndex()), static_cast<quint8>(this->ADC_gain->value()),
                            1, 1,
                            static_cast<quint16>(this->offset_val->value()), nb_output_images, 1,
                            this->cam_fw_type, this->imx->Get_H_period_IMX(this->imx_freq_div->currentIndex(),this->nb_bits_sensor,this->nb_chan,this->DDR_mode_disable->isChecked()));
    this->save_BMP_Thread->reset(this->num_sequence, this->cam_controller->Get_IMX_Freq(this->imx_freq_div->currentIndex()), static_cast<quint8>(this->ADC_gain->value()),
                                 1, 1,
                                 static_cast<quint16>(this->offset_val->value()), nb_output_images);



    if (!this->strThread->isRunning() && !this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning())
    {
      this->strThread->ReleaseCopyBuffer();
    }

    this->mutex.unlock();
    if (this->save_all_TXT->isChecked())
        this->saveThread->start();
    if (this->save_all_BMP->isChecked())
        this->save_BMP_Thread->start();


//     Read temperature of IMX
    this->temperature = this->imx->Get_temperature();
    this->UpdateLastConfiguration();


    this->strThread->start();

    //REQ VIDEO
    this->SendVidReq();

//    Sleep(100);

    this->test_label->setText("Video acquisition in progress...");
    this->test_label->setStyleSheet("");

    if (this->save_all_TXT->isChecked()) {
        this->file_status->setText("File saving in progress...");
        this->file_status->setStyleSheet("");
    }
    else
    {
        this->file_status->setText("");
        this->file_status->setStyleSheet("");
    }

    this->valid_hardware->setText("");
//     qDebug().noquote() << "END";
//
}

void MainWindow::on_Read_last_sequence_clicked() // exclusif à cinema
{

//    this->on_ConfigButton_clicked();

    this->LabelStatusBar->setText("Running...");

    // Update of exposure_time during RUN is forbidden
    this->exposure_doubleSpinBox->setDisabled(true);

    this->offset_val->setDisabled(true);
    this->ADC_gain->setDisabled(true);
    this->binX_val->setDisabled(true);
    this->binY_val->setDisabled(true);
    this->test_mode->setDisabled(true);
    this->Nand_Bypass->setDisabled(true);
    this->read_last_button->setDisabled(true);
    this->window_dim->setDisabled(true);
    this->StartButton->setDisabled(true);
    this->video_settings->setDisabled(true);
    this->AbortButton->setEnabled(true);

    this->imx_freq_div->setDisabled(true);

    this->ConfigButton->setDisabled(true);
    this->save_all_BMP->setDisabled(true);
    this->save_all_TXT->setDisabled(true);

    this->save_BMP_pb->setEnabled(true);
    this->save_TXT_pb->setEnabled(true);

    this->rst_camera->setDisabled(true);

    this->bin2x2->setDisabled(true);
    this->imx_nb_chan->setDisabled(true);

    this->pix_8b->setDisabled(true);

    this->pix_10b->setDisabled(true);
    this->pix_12b->setDisabled(true);

    this->actionProgram->setDisabled(true);
    this->actionProgram_SPI->setDisabled(true);
    this->actionConfigure_CL->setDisabled(true);
    this->actionScript->setDisabled(true);
    this->actionRead_SENSOR_registers->setDisabled(true);//true
    this->test_mode_conf->setDisabled(true);//true


    quint32 log2_bin_y = log2_int(static_cast<quint32>(this->last_seq_binY_val));
    quint32 log2_coadd = log2_int(static_cast<quint32>(this->last_seq_nb_frames_coadded));

    quint8 nb_bits_final = this->last_seq_nb_bits_sensor;



    nb_bits_final = static_cast<quint8>(this->last_seq_nb_bits_sensor + log2(this->last_seq_binX_val) + log2_bin_y + log2_coadd);


    if (nb_bits_final > 8*this->last_seq_spw_nbbytes_per_pixel) {
        nb_bits_final = 8*this->last_seq_spw_nbbytes_per_pixel;
    }

    this->rcv_data_Byte->setCamType(this->cam_fw_type);
    this->rcv_data_Byte->setNbBits(nb_bits_final);
    this->rcv_data_Byte->set_ref_height(static_cast<quint16>(this->last_seq_height / this->last_seq_binY_val));


    int timeout_ms;
    timeout_ms = (5000)*(last_seq_nb_images_val);
//    qDebug().noquote() << "Timeout (ms): " + QString::number(timeout_ms, 10);

//    this->gui_params.spw_camera.setSPW_vid_TimeoutVal(timeout_ms);
    this->gui_params.Cl_camera.setCl_vid_TimeoutVal(timeout_ms);
    this->mutex.lock();
    quint8 nb_output_images = static_cast<quint8>(this->last_seq_nb_images_val / this->last_seq_nb_frames_coadded);

    this->saveThread->reset(this->last_seq_num_sequence,this->last_seq_imx_freq,this->last_seq_ADC_gain,
                            1, 1,this->last_seq_offset_val, nb_output_images, 1,this->cam_fw_type, this->last_seq_imx_h_period);

    this->save_BMP_Thread->reset(this->last_seq_num_sequence, this->last_seq_imx_freq,this->last_seq_ADC_gain,
                                 1, 1,  this->last_seq_offset_val, nb_output_images);

    if (!this->strThread->isRunning() && !this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning())
    {
      this->strThread->ReleaseCopyBuffer();
    }

    this->mutex.unlock();
    if (this->save_all_TXT->isChecked())
        this->saveThread->start();
    if (this->save_all_BMP->isChecked())
        this->save_BMP_Thread->start();


//    this->UpdateLastConfiguration();
//     qDebug().noquote() << "last_temperature (ms): " + QString::number(quint32(this->last_temperature), 10);
//     Read temperature of IMX
    this->temperature = this->imx->Get_temperature(); // bug if assert dont exist
    this->temperature  = this->last_temperature;

    this->strThread->start();
    this->SendReadLast();

    this->test_label->setText("Requesting last sequence stored in nand flash...");
    this->test_label->setStyleSheet("");


    if (this->save_all_TXT->isChecked()) {
        this->file_status->setText("File saving in progress...");
        this->file_status->setStyleSheet("");
    }
    else
    {
        this->file_status->setText("");
        this->file_status->setStyleSheet("");
    }

    this->valid_hardware->setText("");

}

void MainWindow::NOT_Configured() {
    this->LabelStatusBar->setText("Not Configurate");
    QMessageBox msgBox;
    QString msg = "No valid hardware found, channel not open or link not running!";
    msgBox.setText(msg);
    msgBox.exec();
    this->StartButton->setEnabled(false);
    this->ConfigButton->setEnabled(true);
    this->valid_hardware->setText(msg);
    this->valid_hardware->setStyleSheet("font-weight: bold; color: red");

    this->SPW_running->setVisible(true);
    this->SPW_running->setToolTip("Config state");
    this->SPW_running->setStyleSheet("background-color: red;");
}

quint8 MainWindow::get_CMV_freq_div(void) {
//    if (this->cam_fw_type !=CAM_FW_CINEMA) {
//        return static_cast<quint8>(this->CMV_freq_div->value());
//    }
//    else {
        return static_cast<quint8>(this->imx_freq_div->currentIndex());
//    }
}

void MainWindow::streaming(void) {

    this->mutex.lock();

    QString fps_str = QString::number(1000.0 / rcv_data_Byte->getTime()->elapsed()) + " /s";

    if (rcv_data_Byte->getFrame_ID() == 0) {
        fps_str = "";
    }



    QString text;


    text=   "  Temperature\t=   " + QString::number(this->temperature) + " °C\n\n" +
            "  FPS\t\t=   " + fps_str + "\n\n" +
            "  Frame_ID\t=   " + QString::number(rcv_data_Byte->getFrame_ID(), 10) + "\n\n" +
            "  Width\t\t=   " + QString::number(rcv_data_Byte->getWidth(), 10) + "\n\n" +
            "  Height\t\t=   " + QString::number(rcv_data_Byte->getHeight(), 10);



    this->temperature_label->setText(text);
    rcv_data_Byte->getTime()->restart();

    this->pixmap_img = rcv_data_Byte->toPixMap(this->SensorIsColor);
    this->mutex.unlock();

    this->screen_lbl->setPixmap(pixmap_img.scaled(scrollArea->width(),scrollArea->height(),Qt::KeepAspectRatio));

    // qDebug().noquote() << "After setPixmap";

    if (this->save_all_BMP->isChecked()) {
        req_bmp_file();
    }
    if (this->save_all_TXT->isChecked()) {
        this->req_txt_file();
    }
}

// Test mode check box
void MainWindow::on_test_mode_stateChanged()
{
    if (this->test_mode->isChecked()) {
        this->PrintStatusBar("Test mode enabled");
//        if (this->cam_fw_type == CAM_FW_CINEMA)
//          this->test_mode_conf->setEnabled(true);
          this->imx->EnableTestMode(true);
          this->offset_val->setValue(0); // Shall be set to 0 when using pattern generator

        }
    else {
        this->PrintStatusBar("Test mode disabled");
//        if (this->cam_fw_type == CAM_FW_CINEMA){
//          this->test_mode_conf->setEnabled(false);
          this->imx->EnableTestMode(false);
//        }
    }
}

// Binning 2x2 check box
//void MainWindow::on_bin2x2_stateChanged()
//{

//    if (this->bin2x2->isChecked()){
//        this->binX_val->setValue(2);
//        this->binY_val->setValue(2);
//    }
//    else {
//        this->binX_val->setValue(1);
//        this->binY_val->setValue(1);
//    }
//}

// Pixel resolution radio buttons
void MainWindow::on_pix_8b_clicked()   // a adapter
{
    this->nb_bits_sensor = 8;
    if (this->configurate) {

    this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
    this->imx->config( this->nb_chan,    // int   nb_chan      , // Number of data channels
                          8, this->DDR_mode_disable->isChecked(), this->xtrig_en);    // int   nb_bits       // 8, 10 or 12

    this->cam_controller->config_SENS_CTL(this->cam_fw_type, this->test_mode->isChecked(), 8, 0);
    this->calib();

    }


    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);



    this->compute_maximum_width();

    this->PrintStatusBar("Pixel resolution set to 8 bits");
    this->compute_FPS();
}


void MainWindow::on_DDR_mode_stateChanged()
{
   if (this->DDR_mode_disable->isChecked())
       this->PrintStatusBar("Sensor DDR mode disabled");
   else
       this->PrintStatusBar("Sensor DDR mode enabled");

   this->imx->configFREQ(this->DDR_mode_disable->isChecked());

   double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
   if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);

   this->compute_FPS();
}

void MainWindow::on_pix_10b_clicked()
{
    this->nb_bits_sensor = 10;
    if (this->configurate) {

            this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
            this->imx->config( this->nb_chan,    // int   nb_chan      , // Number of data channels
                                  nb_bits_sensor, this->DDR_mode_disable->isChecked(), this->xtrig_en);    // int   nb_bits       // 8, 10 or 12
         this->cam_controller->config_SENS_CTL(this->cam_fw_type, this->test_mode->isChecked(), nb_bits_sensor, 0);
        this->calib();
    }


    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);


    this->PrintStatusBar("Pixel resolution set to 10 bits");
    this->compute_maximum_width();
    this->compute_FPS();
}

void MainWindow::on_pix_12b_clicked()
{
    this->nb_bits_sensor = 12;
    if (this->configurate) {

        this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
        this->imx->config( this->nb_chan,    // int   nb_chan      , // Number of data channels
                              nb_bits_sensor, this->DDR_mode_disable->isChecked(), this->xtrig_en);    // int   nb_bits       // 8, 10 or 12

         this->cam_controller->config_SENS_CTL(this->cam_fw_type, this->test_mode->isChecked(), nb_bits_sensor, 0);
        this->calib();
    }

    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);

    this->PrintStatusBar("Pixel resolution set to 12 bits");

    this->compute_maximum_width();
    this->compute_FPS();
}

void MainWindow::on_video_infinite_stateChanged() {
    if (this->video_infinite->isChecked()) {
        this->nb_images_val->setDisabled(true);
        if (this->cam_fw_type == CAM_FW_CO2M || this->cam_fw_type == CAM_FW_CINEMA || this->cam_fw_type == CAM_FW_CASPEX_IR){
            this->nb_images_val->setMinimum(0);
            this->nb_images_val->setValue(0);
        }
        else {
            this->nb_images_val->setMaximum(255);
            this->nb_images_val->setValue(255);
        }

        this->nb_frames_coadded->setDisabled(true);
        this->nb_frames_coadded->setValue(1);
    }
    else {
        this->nb_images_val->setEnabled(true);
        if (this->cam_fw_type == CAM_FW_CO2M || this->cam_fw_type == CAM_FW_CINEMA || this->cam_fw_type == CAM_FW_CASPEX_IR){
            this->nb_images_val->setMinimum(1);
        }
        else {
            this->nb_images_val->setMaximum(254);
        }
        this->nb_frames_coadded->setEnabled(true);
    }

}

void MainWindow::on_binX_val_valueChanged()
{
    quint8 bin_X_val = static_cast<quint8>(this->binX_val->value());

    //update CL buffersize
    this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);

    if ((quint32(this->width->value())/quint32(bin_X_val) < HEADER_SIZE*NB_PXL_PER_CC )){
        this->width->setMinimum(int(HEADER_SIZE*NB_PXL_PER_CC) * bin_X_val);
    }else
        this->width->setMinimum(2 * bin_X_val);

    if (this->width->value() < this->width->minimum()) {
        this->width->setValue(this->width->minimum());
        this->on_width_valueChanged(this->width->minimum());
    }

    this->compute_maximum_width();

    if (this->configurate)
    {
        this->compute_FPS();
        this->config_Binning(static_cast<quint8>(this->binX_val->value()),
                             static_cast<quint8>(this->binY_val->value()),
                             static_cast<quint8>(this->bin_mode_val->currentIndex()));
    }
    this->PrintStatusBar("Binning X set to " + QString::number(bin_X_val));
}

void MainWindow::on_binY_val_valueChanged()
{
    quint8 bin_Y_val = static_cast<quint8>(this->binY_val->value());

    //update CL buffersize
    this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);

    this->height->setSingleStep(bin_Y_val);
    this->height->setMinimum(bin_Y_val);

    if ( this->cam_fw_type == CAM_FW_CINEMA){
        this->height->setMaximum(MAX_HEIGHT_IMX253 - (MAX_HEIGHT_IMX253 % bin_Y_val));
    }
    else {
        this->height->setMaximum(MAX_HEIGHT_IMX990 - (MAX_HEIGHT_IMX990 % bin_Y_val));
    }

    this->height->setValue(this->height->value() - (this->height->value() % bin_Y_val));

    this->compute_maximum_width();

    this->compute_FPS();

    if (this->configurate)
        this->config_Binning(static_cast<quint8>(this->binX_val->value()),
                             static_cast<quint8>(bin_Y_val),
                             static_cast<quint8>(this->bin_mode_val->currentIndex()));

    this->PrintStatusBar("Binning Y set to " + QString::number(bin_Y_val));
}



void MainWindow::on_bin_mode_val_valueChanged() {

    this->config_Binning(static_cast<quint8>(this->binX_val->value()),
                         static_cast<quint8>(this->binY_val->value()),
                         static_cast<quint8>(this->bin_mode_val->currentIndex()));

    this->PrintStatusBar("Binning Mode set to " + this->bin_mode_val->currentText());
}

void MainWindow::compute_maximum_width() {


    double sens_in_freq = this->cam_controller->Get_IMX_Freq(this->imx_freq_div->currentIndex());
    quint32 image_size;
    if ( this->cam_fw_type == CAM_FW_CINEMA){
    image_size = MAX_WIDTH_IMX253;
    }else {
     image_size = MAX_WIDTH_IMX990;
    }

    // This variable shall be coded with 32 bits!!
//    quint32 max_width_loc = static_cast<quint32>( image_size/this->nb_chan / sens_in_freq * this->strThread->get_DDR_bandwidth(static_cast<quint8>(this->nb_frames_coadded->value())) *
//                                              this->binX_val->value() * this->binY_val->value());  //
    double DDR3_Bw= double(this->strThread->get_DDR_bandwidth(static_cast<quint8>(this->nb_frames_coadded->value())));
//    quint32 max_width_loc = static_cast<quint32>(

//    qDebug().noquote()<< "NB bit" + QString::number(this->nb_bits_sensor,10);
    double IMX_Bw = this->imx->Get_IMX_Bandwidth( this->nb_chan ,this->nb_bits_sensor);
//    qDebug().noquote()<< "IMX_Bw" + QString::number(quint32(IMX_Bw),10);
   quint32 max_width_loc = static_cast<quint32>((DDR3_Bw/IMX_Bw)*image_size*this->binX_val->value());
    if (max_width_loc > image_size) {
        max_width_loc = image_size;
    }


    max_width_loc -= (max_width_loc % 8);
    this->width->setMaximum(static_cast<quint16>(max_width_loc));

    if (this->width->value() > this->width->maximum()) {
        this->width->setValue(this->width->maximum());
        this->on_width_valueChanged(this->width->maximum());
    }
}

void MainWindow::on_width_valueChanged(int width)
{
    this->PrintStatusBar("Update width value");
//    this->gui_params.spw_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXLAST_AD0, (static_cast<quint16>(width + this->offset_x->value() -1)));
    this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXLAST_AD0, (static_cast<quint16>(width + this->offset_x->value() -1)));

    this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);

    this->compute_FPS();
}

void MainWindow::on_height_valueChanged(int arg1)
{

    this->PrintStatusBar("Update height value");
    quint16 height = static_cast<quint16>(arg1);

//    this->gui_params.spw_camera.send_TC(2, SENS_CTL_ba | CMVCTL_ROWWIDTH_AD0, (height-1));
    this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | CMVCTL_ROWWIDTH_AD0, (height-1));
    //uptdate CL_buffer size
    this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);

   //VMAX
   this->imx->Set_VMAX(height);

   // update exposure time
   this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());

    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);

    this->compute_FPS();
}

void MainWindow::on_offset_x_valueChanged(int arg1)
{
    this->PrintStatusBar("Update offset x value");
    quint16 offset_x = static_cast<quint16>(arg1);
    quint16 width = static_cast<quint16>(this->width->value());

//    this->gui_params.spw_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXSTART_AD0, offset_x);

//    this->gui_params.spw_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXLAST_AD0, (width + offset_x -1));

    this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXSTART_AD0, offset_x);

    this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | CMVCTL_PIXLAST_AD0, (width + offset_x -1));

    //ROI la gestion du fenetrage ne doit elle pas se faire via le capteur ??


}

void MainWindow::on_offset_y_valueChanged(int arg1)
{
    this->PrintStatusBar("Update offset y value");
    quint16 offset_y = static_cast<quint16>(arg1);

//    this->gui_params.spw_camera.send_TC(2, SENS_CTL_ba | CMVCTL_ROWSTART_AD0, offset_y);
    this->gui_params.Cl_camera.send_TC(2, SENS_CTL_ba | CMVCTL_ROWSTART_AD0, offset_y);

    this->imx->Set_Offset_y(offset_y);
}

void MainWindow::on_ADC_gain_valueChanged(int arg1)
{
    this->PrintStatusBar("Update ADC gain value");
    this->imx->Set_ADC(static_cast<quint32>(10*arg1));
}

void MainWindow::closeEvent (QCloseEvent *event)
{

    this->Quit();
    event->ignore();
}

void MainWindow::on_AbortButton_clicked()
{
    this->LabelStatusBar->setText("Abort");
    //qDebug().noquote()  << "Click ABORT";

    this->Abort_Video();

    this->saveThread->set_stop(true);
    this->save_BMP_Thread->set_stop(true);

    this->AbortButton->setEnabled(false);
}

void MainWindow::on_exposure_doubleSpinBox_valueChanged(double exposure_time_ms)
{
    this->PrintStatusBar("Update exposure_time to " + QString::number(exposure_time_ms) + " ms");    

    this->compute_FPS();

    this->cam_controller->Set_Exp_imx(exposure_time_ms, quint8(this->imx_freq_div->currentIndex()),this->nb_bits_sensor,this->nb_chan, quint32(this->height->value()), this->DDR_mode_disable->isChecked());
}

void MainWindow::req_bmp_file() {
    this->mutex.lock();
    this->save_BMP_Thread->add_pixmap(this->pixmap_img);
    this->mutex.unlock();
}

void MainWindow::req_txt_file() {

    this->mutex.lock();
    this->saveThread->add_cu_frame(this->rcv_data_Byte);
    this->mutex.unlock();


}

void MainWindow::on_save_BMP_pb_clicked()
{
    this->save_BMP_Thread->set_stop(false);
    this->req_bmp_file();
    this->save_BMP_Thread->start();
}

void MainWindow::on_actionProgram_triggered()
{
    ProgramWindow window(this->cam_hw_type, ".bit");

    window.exec();

}

void MainWindow::on_actionProgram_SPI_triggered()
{
    ProgramWindow window(this->cam_hw_type, ".mcs");

    window.exec();

}

void MainWindow::on_actionScript_triggered(void)
{
    this->PrintStatusBar("Run script");

    //    QFileDialog dialog(this);
    //    dialog.setFileMode(QFileDialog::AnyFile);
    //    dialog.setNameFilter(tr("Script 3D PLUS (*.3Dscript);; All files (*.*)"));
    //    dialog.setOption(QFileDialog::ShowDirsOnly);
    //    dialog.setViewMode(QFileDialog::List);
    //    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    //    QStringList fileNames;
    //    if(dialog.exec())
    //    {
    //        fileNames = dialog.selectedFiles();
    QString path = "0";
    // script a gerer
    scriptWindow = new script(this->cam_controller, this->imx, &this->gui_params.Cl_camera, path,  this->cam_fw_type, this->SensorIsColor, false);
    scriptWindow->exec();

    if (this->gui_params.Cl_camera.get_Cl_Running()== true) {
        this->updateAllParameters();
    }

    //    }
    //    else
    //        dialog.reject();

}

void MainWindow::on_action_exportScript_triggered(void)
{
    this->PrintStatusBar("Save script");

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Script 3D PLUS (*.3Dscript);; All files (*.*)"));
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(".3Dscript");
    QStringList fileNames;
    if(dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        this->gui_params.Cl_camera.save_script_file(fileNames.at(0));
    }
    else
        dialog.reject();

}

void MainWindow::on_actionExit_triggered()
{
    //qDebug().noquote()  << "Clic EXIT";

    this->Quit();
}

void MainWindow::Quit(void) {

    QMessageBox::StandardButton resBtn = QMessageBox::question( this, this->windowTitle(),
                                                                tr("Are you sure you want to exit the application?\n"),
                                                                QMessageBox::No | QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {

        if (this->strThread->isRunning())
            this->on_AbortButton_clicked();

        while (this->strThread->isRunning());

        QApplication::quit();
    }

}


void MainWindow::on_test_configuration_clicked()
{
    //qDebug().noquote()  << "Clic test configuration";

    //    QByteArray cmv_regs = this->cmv4000->get_CMV_regs();
    //qDebug().noquote()  << "PRINT CMV REGS";
    //print_data_Bytes(cmv_regs);
    QByteArray regs = this->imx->get_test_regs();

    Test_mode_configuration window(regs);

    //window.show();
    window.exec();

    if(window.close()){
        //qDebug().noquote()  << "IMX dialog exit";
        QByteArray updated_regs = window.get_test_mode_regs_values();
        this->imx->set_test_regs(updated_regs);
        //print_data_Bytes(updated_regs);

        // read offset and test mode enable
        this->offset_val->setValue(this->imx->Get_Offset());

        this->test_mode->setChecked(this->imx->Get_Test_Mode());

    }
}


void MainWindow::on_actionRead_SENS_registers_triggered()
{
    //qDebug().noquote()  << "Clic Read CMV regs";

        QByteArray imx_regs = this->imx->get_IMX_regs();

        // qDebug().noquote()  << "PRINT IMX REGS";
        // print_data_Bytes(imx_regs);

        IMX_read_registers window(imx_regs);

        //window.show();
        window.exec();

        if(window.close()){
            //qDebug().noquote()  << "CMV dialog exit";
            QByteArray updated_regs = window.get_imx_regs_values();
            this->imx->set_IMX_regs(updated_regs);
            //print_data_Bytes(updated_regs);
        }
}


void MainWindow::on_actionSend_TC_TM_triggered()
{
    //qDebug().noquote()  << "Click Send TC/TM";

    // Clear display
    this->window_send_TMTC->set_TM_params("",true);
    this->window_send_TMTC->set_TC_params("Waiting for user input ...",false);

    this->window_send_TMTC->exec();
}


void MainWindow::transmit_TC()
{
    quint8 TC_data[TMTC_SIZE_MAX];
    quint8 nb_data;

    //qDebug().noquote()  << "send TC clicked in QDialog!";
    this->TC_params = this->window_send_TMTC->get_TC_params();
    //qDebug().noquote()  << "TC_addr    = 0x" + QString::number(TC_params.addr,16);
    //qDebug().noquote()  << "TC_nb_data = 0x" + QString::number(TC_params.data_pkts.size()/2,16);
    //qDebug().noquote()  << "TC data    = 0x" + TC_params.data_pkts;

    QByteArray TC_regs;
    TC_regs.clear();

    // Convert QString to QByteArray
    TC_regs = QByteArray::fromHex(TC_params.data_pkts.toLocal8Bit());
    TC_regs.data();

    //qDebug().noquote()  << "Print TC_regs";
    //print_data_Bytes(TC_regs);

    // Convert QByteArray to quint8 Array
    nb_data = static_cast<quint8>(TC_regs.size());
    for (quint8 i =0; i < nb_data; i++) {
        TC_data[i] = static_cast<quint8>(TC_regs.at(i));
    }

    if(nb_data != 0) {
//        this->gui_params.spw_camera.send_TC(nb_data, TC_params.addr, TC_data);
        this->gui_params.Cl_camera.send_TC(nb_data, TC_params.addr, TC_data);
    }

    this->window_send_TMTC->set_TC_params("Number of bytes written : " + QString::number(nb_data,10),0);

}

void MainWindow::show_TM()
{
    this->TM_params = this->window_send_TMTC->get_TM_params();
    //qDebug().noquote()  << "TM_addr    = 0x" + QString::number(TM_params.addr,16);
    //qDebug().noquote()  << "TM_nb_data = 0x" + QString::number(TM_params.nb_data,16);

    TM_regs.clear();
    TM_regs = this->gui_params.Cl_camera.send_TM(TM_params.nb_data,TM_params.addr);

    for (quint32 i =0; i < static_cast<quint32>(TM_regs.size()); i++) {
        // this->window_send_TMTC->set_TM_params("Byte " + QString::number(i,10).rightJustified(4, ' ') + "  =  0x" + QString::number((quint8)TM_regs.at(i), 16).rightJustified(2, '0') + "     |    Addr : 0x" + QString::number(TM_params.addr+i,16).rightJustified(4, '0'),false);
        this->window_send_TMTC->set_TM_params( "Addr : 0x" + QString::number(TM_params.addr+i,16).rightJustified(4, '0') + "     |     Data =  0x" + QString::number(static_cast<quint8>(TM_regs.at(static_cast<int>(i))), 16).rightJustified(2, '0'),false);

    }
}

void MainWindow::on_actionConfigure_CL_triggered()
{
    // qDebug().noquote()  <<"first !" + QString::number(TapNumber,10);
    CLdialog window_Cl(this->TapNumber);
//   window_Cl.Set_Cl_params(0);
   window_Cl.exec();

   if(window_Cl.close()){
       //qDebug().noquote()  << "CameraLink dialog box exit";

       CL_params CL_params = window_Cl.Get_Cl_params();
       this->mutex.lock();
       this->TapNumber = CL_params.TapNumber;
       this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);
       this->gui_params.Cl_camera.UpdateTapNumber(this->TapNumber, this->configurate);
       this->mutex.unlock();
   }
   compute_FPS();
}

void MainWindow::on_actionAbout_triggered()
{
    //qDebug().noquote()  << "Click About";

    aboutWindow window(egse_version);

    QString project_name = "CO2M";
    if (this->cam_fw_type == CAM_FW_3DIPCC0746){
        project_name = "3DIPCC0746";
    }
    else if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
        project_name = "3DIPCC0802";
    }else if (this->cam_fw_type == CAM_FW_CASPEX_IR) {
            project_name = "CASPEX_IR";
    }else
        project_name = "3DCM800";

    if (this->configurate) {
        quint32 fpga_version = this->gui_params.Cl_camera.send_TM(VERSION_STS_AD);

        window.set_fpga_version(project_name,fpga_version);
    }

    window.exec();

}

void MainWindow::on_nb_images_val_valueChanged(int arg1)
{
    this->PrintStatusBar("Setting number of images to " + QString::number(arg1,10));

    // Number of frames to capture in the sequence
    this->gui_params.Cl_camera.send_TC(SENS_CTL_ba | NB_IMAGE_AD, static_cast<quint8>(arg1));
}


void MainWindow::on_nb_frames_coadded_val_valueChanged(int value)
{
    this->compute_maximum_width();
    this->compute_FPS();

    // Number of frames to capture in the sequence
//    if (this->cam_fw_type == CAM_FW_CO2M||this->cam_fw_type == CAM_FW_CINEMA) {
        this->PrintStatusBar("Setting number of frames for coadding to " + QString::number(value, 10));
        this->gui_params.Cl_camera.send_TC(MC_DDC_ba, static_cast<quint8>(value)-1);
//    }
}


void MainWindow::on_fps_divider_valueChanged(int fps_divider)
{

    this->PrintStatusBar("Setting frame rate divider to " + QString::number(fps_divider,'f',0));

    // ecrire registre amc_imx
    this->gui_params.Cl_camera.send_TC(1, SENS_CTL_ba | SENSCTL_FRAME_RATE_AD, quint8(fps_divider));

    this->compute_FPS();
}

void MainWindow::on_time_interval_val_valueChanged(double interframe_ms)   // In ms
{
        double imx_in_freq_MHz;
        if (this->imx_freq_div->currentIndex() == 0)
            imx_in_freq_MHz = 74.25;
        else if (this->imx_freq_div->currentIndex() == 1)
            imx_in_freq_MHz = 54;
        else
            imx_in_freq_MHz = 37.125;
        // Time interval between 2 frames of a sequence
        double sysclk_per_ms   = 1 / (1000 * imx_in_freq_MHz);    // Period, in ms
        quint32 interval       = static_cast<quint32>(interframe_ms / sysclk_per_ms);
        this->PrintStatusBar("Setting frame period to " + QString::number(interframe_ms,'f',0) + " ms, register = " + QString::number(interval));
//        qDebug().noquote() << "SETTING nb tick of interframe : " + QString::number(interval,10);
//          qDebug().noquote() << "--------------------------";
        this->gui_params.Cl_camera.send_TC(4, SENS_CTL_ba | INTER_IMAGE_AD0, interval);
        this->compute_FPS();

    //  ui->frame_rate_val->setValue(1000/arg1);
}

void MainWindow::compute_FPS(void) {

    double req_fps;
    double sensor_fps=0;
    double sens_in_freq;
//    if (this->cam_fw_type != CAM_FW_CINEMA){
//        sens_in_freq = this->cam_controller->get_cmv_in_freq(this->cam_fw_type, this->get_CMV_freq_div());
//    }
//    else
//    {
         sens_in_freq = this->cam_controller->Get_IMX_Freq(this->imx_freq_div->currentIndex());
//    }

//    if (this->cam_fw_type != CAM_FW_CINEMA){
//        req_fps = 1000/this->time_interval_val->value();
//    }else {

        sensor_fps = this->imx->Get_FPS(quint32(this->height->value()), sens_in_freq, this->nb_chan, this->nb_bits_sensor, this->DDR_mode_disable->isChecked());
        req_fps = sensor_fps/this->fps_divider->value();

//    }

    if (this->xtrig_en == true)
        req_fps = 1000 /this->time_interval_val->value();

    QString fps = "Requested Frame rate : " + QString::number(req_fps,0,2) + " FPS\n";
    this->FPS->setText(fps);

    double sensor_fps_val ;
    //attention plus vrai si mode XTRIG
    sensor_fps_val = sensor_fps;

    fps = "Max Sensor Frame rate : " + QString::number(sensor_fps_val,0,2) + " FPS\n";

    double nb_pixels = static_cast<double>(this->width->value() / this->binX_val->value() * (this->height->value() / this->binY_val->value() + 1));

    double ddr_fps_val = this->strThread->get_DDR_bandwidth(static_cast<quint8>(this->nb_frames_coadded->value()))*1000000/nb_pixels;
//    if (this->cam_fw_type == CAM_FW_CINEMA) {
        fps.append("Max DDR3 Frame rate : " + QString::number(ddr_fps_val,0,2) + " FPS\n");
//    }

    double fmc_fps_val = (5*1000000)/nb_pixels; // 5 MPxl/s write only
//    if (this->cam_fw_type == CAM_FW_CINEMA) {
            fps.append("Max Nand Flash Frame rate : " + QString::number(fmc_fps_val,0,2) + " FPS\n");
//     }

//    quint32 nb_bytes = this->spw_nbbytes_per_pixel * this->rcv_data_Byte->getCMVHeaderSize() + this->spw_nbbytes_per_pixel * nb_pixels;
//    quint32 nb_bits = 10 * nb_bytes;  // 10 bits per Byte in SpaceWire protocol
////    double time_frame = nb_bits * (this->spw_txdiv_cnt+1) / (this->SpwClk_freq*1000000);  // In seconds

//    double spw_fps_val = 1/time_frame;
//    fps.append("Max SPW Frame rate : " + QString::number(spw_fps_val,0,2) + " FPS");
    double Cl_freq = 80*1000000; //CL 80 MHz
    double cl_fps_val = 1/(nb_pixels/(this->TapNumber*Cl_freq));
    fps.append("Max CL Frame rate : " + QString::number(cl_fps_val,0,2) + " FPS");
    this->max_FPS->setText(fps);

    this->max_FPS->setStyleSheet("color: black");


    if (cl_fps_val < req_fps || (fmc_fps_val < req_fps && !this->Nand_Bypass->isChecked())) {
        this->max_FPS->setStyleSheet("color: orange");
    }



    if (sensor_fps_val < req_fps || ddr_fps_val < req_fps)
        this->max_FPS->setStyleSheet("color: red");
    double IMXBw =  this->imx->Get_IMX_Bandwidth( this->nb_chan ,this->nb_bits_sensor);
    QString pixel_rate = "Sensor = " + QString::number(quint32(IMXBw)) + " Mpixels/s\n";

    pixel_rate.append("DDR3 = " + QString::number(this->strThread->get_DDR_bandwidth(static_cast<quint8>(this->nb_frames_coadded->value()))) + " Mpixels/s");


    this->pixel_rate_lbl->setText(pixel_rate);
}

void MainWindow::on_rst_camera_clicked()
{
    qDebug().noquote()  << "Reset sensor";
    //quint8 reset_cmd = 0x1;
    //this->gui_params.spw_camera.send_TC(SPW_RESET_AD, reset_cmd);
    this->cam_controller->reset_sensor();

    // this->test_mode->setChecked(this->imx->Get_Test_Mode());

    this->StartButton->setEnabled(false);
    this->ConfigButton->setEnabled(true);
}

void MainWindow::on_save_TXT_pb_clicked()
{


    this->saveThread->set_stop(false);
    this->req_txt_file();
    this->saveThread->start();
}

void MainWindow::on_ConfigButton_clicked()
{

    Sleep(100); // wait before XCE falling edge
    this->LabelStatusBar->setText("Configuration...");
    this->ConfigButton->setEnabled(false);
    this->StartButton->setEnabled(false);

    if (this->gui_params.Cl_camera.get_Cl_Running()== true) {
      this->gui_params.Cl_camera.CameraLink_close();
    }

    // qDebug() << "CL OPEN";
//    this->configurate = this->gui_params.spw_camera.config_CU();
    if (this->gui_params.Cl_camera.get_Cl_Running()==false) {
        this->configurate = this->gui_params.Cl_camera.CameraLink_open();
       // qDebug() << "After Open";
    }

//    qDebug() << "Tap Number" + QString::number(TapNumber,10);
    this->gui_params.Cl_camera.Set_Cl_buffer_params(quint32(this->width->value()),quint32(this->height->value()),  quint8 (this->binX_val->value()), quint8 (this->binY_val->value()), this->TapNumber);
//    qDebug() << "------------";

    this->gui_params.Cl_camera.Update_ICD(this->TapNumber);
    this->rcv_data_Byte->setNbBytes_per_pix(4); // 24 bits + 8 bits de phantom induits par PCIe
   // qDebug() << "After Update ICD";

    //configure MUX to select CL
    quint32 addr = (0x202);
    this->gui_params.Cl_camera.send_TC(addr,0x0);

    //qDebug() << "config" + QString::number(this->configurate,10);
    // Write FOT
        this->sensor = this->imx->Get_SensorType();
        if (this->sensor == 5)
            this->SensorIsColor = 1;
        else {
            this->SensorIsColor = 0;
        }
        this->cam_controller->reset_sensor();

        Sleep(500);

        this->cam_controller->clk_config(this->cam_fw_type, quint8(this->imx_freq_div->currentIndex()), 0);
        this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
        this->imx->config( this->nb_chan,    // int   nb_chan      , // Number of data channels
                              this->nb_bits_sensor,
                              this->DDR_mode_disable->isChecked(), this->xtrig_en);    // int   nb_bits      , // 8, 10 or 12


      this->imx->enable_PGMODE(test_mode->isChecked());

    // Set frame_tag in order to identify each frame
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD0, 0x0);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD1, 0x1);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD2, 0x2);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD3, 0x3);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD4, 0x4);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD5, 0x5);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD6, 0x6);
    this->gui_params.Cl_camera.send_TC( SENS_CTL_ba | FRAME_TAG_AD7, 0x7);

     if (this->cam_fw_type == CAM_FW_CO2M ){ // configure refresh DDR2
         this->gui_params.Cl_camera.send_TC(BYPASS_REG_AD, (2 << MODE_gp));
     }


    on_NAND_bypass();
//    if (this->Nand_Bypass->isChecked()) {
//        this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, static_cast<quint8>(this->fmc_bypass << 3));
//    }
//    else
//        this->gui_params.Cl_camera.send_TC(FLASH_CTL_ba | APB_MODE_REG, MCFMC_READ_NOMINAL_MODE);


//     Check that Bypass mode is disabled
//     quint8 mode_reg_sts = this->gui_params.Cl_camera.send_TM(FLASH_CTL_ba |APB_MODE_REG);
//     if( (mode_reg_sts >> 3) == 1){
//         qDebug().noquote()  << "Warning : Bypass FLASH mode is ENABLED!";
//     }

    // a décommenter
    // this->gui_params.spw_camera.send_TC(SENS_CTL_ba | CMVCTL_CTL_AD, CMVCTL_CTL_CLR_ERROR_gm);
    // Change SDRAM Buffer size
//     if (this->cam_fw_type != CAM_FW_CO2M && this->cam_fw_type != CAM_FW_CINEMA){
//         this->gui_params.Cl_camera.send_TC(4, MC_DDC_ba | END_AD0, 0xFFFFFFFF);
//     }

    this->UpdateSensor();
     this->mutex.lock();
    if (this->sensor == 0)
    {
        this->CU_ConfigError();
    }
     this->mutex.unlock();

    if (this->configurate)
        this->UpdateButton();
    else
        this->NOT_Configured();


    Enable_IHM_param(this->configurate);

    //   this->imx->enable_standby_mode(false);

}

void MainWindow::updateAllParameters(void) {




    this->on_IMX_nb_chan_valueChanged();
    this->on_IMX_freq_div_valueChanged();

    this->on_binX_val_valueChanged();
    this->on_binY_val_valueChanged();
    this->on_bin_mode_val_valueChanged();
    this->on_nb_images_val_valueChanged(this->nb_images_val->value());
    this->on_offset_x_valueChanged(this->offset_x->value());
    this->on_offset_y_valueChanged(this->offset_y->value());
    this->on_height_valueChanged(this->height->value());
    this->on_width_valueChanged(this->width->value());
    this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
    this->on_time_interval_val_valueChanged(this->time_interval_val->value());
    this->on_fps_divider_valueChanged(this->fps_divider->value());
    this->on_nb_frames_coadded_val_valueChanged(this->nb_frames_coadded->value());
    this->on_video_infinite_stateChanged();

    if (this->pix_12b->isChecked())
        this->on_pix_12b_clicked();

    if (this->pix_10b->isChecked())
        this->on_pix_10b_clicked();


        if (this->pix_8b->isChecked())
            this->on_pix_8b_clicked();

        if (this->DDR_mode_disable->isChecked())
           this->on_DDR_mode_stateChanged();


    this->on_test_mode_stateChanged();

    this->test_mode->setChecked(this->imx->Get_Test_Mode());

    this->on_ADC_gain_valueChanged(this->ADC_gain->value());
    this->on_offset_val_valueChanged(this->offset_val->value());


//    if (this->cam_fw_type == CAM_FW_3DIPCC0746)
//        this->on_bin2x2_stateChanged();

}

void MainWindow::UpdateButton(void)
{
    this->LabelStatusBar->setText("Configuration done");
    this->ConfigButton->setEnabled(true);
    this->mutex.lock();
//    bool IsSPWRunning_i = this->gui_params.spw_camera.get_SPW_Running();
    bool IsClRunning_i = this->gui_params.Cl_camera.get_Cl_Running();
    this->mutex.unlock();
    if (this->configurate)
    {

        this->actionRead_SENSOR_registers->setEnabled(true);
        this->actionSend_TC_TM->setEnabled(true);
        this->actionScript->setEnabled(true);

        this->Error->setStyleSheet("background-color: green;");
        if (IsClRunning_i)
        {
            this->StartButton->setEnabled(true);
            this->SPW_running->setStyleSheet("background-color: green;");

            this->updateAllParameters();

            // DDR mode is available only when SpwClk_freq = SysClk_freq
//            if(this->cam_fw_type == CAM_FW_CO2M ||this->cam_fw_type == CAM_FW_CINEMA ){
//                this->gui_params.Cl_camera.send_TC(TXDIVCNT_AD, this->spw_txdiv_cnt);
//                this->gui_params.Cl_camera.send_TC(NBBYTES_AD, this->spw_nbbytes_per_pixel);
//            }
//            else {
//                this->gui_params.Cl_camera.send_TC(TXDIVCNT_AD, 0x80 | this->spw_txdiv_cnt); // DDR mode always enabled
//            }
        }
        else
            this->SPW_running->setStyleSheet("background-color: red;");
        this->Error->setVisible(true);
        this->SPW_running->setVisible(true);
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Not configurate");
        msgBox.exec();
    }
    this->valid_hardware->setText("");


}

void MainWindow::TOReceiveSPW(void)
{
    QMessageBox msgBox;
    msgBox.setText("Time OUT !!");
    msgBox.exec();
    on_AbortButton_clicked();
}

void MainWindow::SetError(void)
{
    this->Error->setStyleSheet("background-color: red;");
}

void MainWindow::UpdateSensor(void)
{
    this->mutex.lock();
    if (this->sensor == 2)
        this->PrintStatusBar("Found CMV2000");
    else if (this->sensor == 3 | this->sensor == 5) {
        this->PrintStatusBar("Found IMX");
    }
    else if (this->sensor == 4) {
        this->PrintStatusBar("Found CMV4000");
    }
    else
    {
        this->mutex.unlock();
        this->PrintStatusBar("No sensor found!!!!!!");
        on_AbortButton_clicked();
        this->mutex.lock();
    }
    this->mutex.unlock();

}

void MainWindow::CU_ConfigError(void)
{

    this->test_label->setText("Error when configuring the CU !\n");
    this->test_label->setStyleSheet("font-weight: bold; color: red");

    this->ConfigButton->setEnabled(true);
    this->exposure_doubleSpinBox->setEnabled(true);
    this->video_settings->setEnabled(true);
    this->window_dim->setEnabled(true);
    this->rst_camera->setEnabled(true);
    this->CMV_freq_div->setEnabled(true);

    this->bin2x2->setEnabled(true);
    this->pix_10b->setEnabled(true);
    this->pix_12b->setEnabled(true);
    this->DDR_mode_disable->setEnabled(false); // true

    this->actionProgram->setEnabled(true);
    this->actionProgram_SPI->setEnabled(true);
    this->actionConfigure_CL->setEnabled(true);
    this->actionScript->setEnabled(true);
    this->actionRead_SENSOR_registers->setEnabled(true);

    this->AbortButton->setEnabled(false);
    this->StartButton->setEnabled(true);
}

void MainWindow::EndSequence(void) {

   //if (this->cam_fw_type == CAM_FW_CINEMA) {
   //  this->imx->enable_standby_mode(true);
   //  this->imx->disable_master_mode();
   //}

    this->num_sequence++;
    this->num_image_bmp = 0;
}

void MainWindow::EndThread(void)
{

    this->mutex.lock();
    if (this->configurate)
    {
        if (!this->strThread->isRunning()) {
            this->test_label->setText("Video acquisition is DONE !\n");
            this->test_label->setStyleSheet("font-weight: bold; color: green");
        }

        if ( this->save_all_TXT->isChecked() || this->save_all_BMP->isChecked()) {
            if (!this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning()) {
                this->file_status->setText("File saving is DONE !\n");
                this->file_status->setStyleSheet("font-weight: bold; color: green");
            }
        }
    }

    this->mutex.unlock();

/*    qDebug().noquote() << "strThread  " + QString::number(!this->strThread->isRunning());
    qDebug().noquote() << "saveThread  "  + QString::number(!this->saveThread->isRunning());
    qDebug().noquote() << "save_BMP_Thread  " + QString::number(!this->save_BMP_Thread->isRunning());*/

    if (!this->strThread->isRunning() && !this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning())
    {

        this->LabelStatusBar->setText("Ready");

        this->ConfigButton->setEnabled(true);
        this->exposure_doubleSpinBox->setEnabled(true);
        this->video_settings->setEnabled(true);
        this->window_dim->setEnabled(true);
        this->rst_camera->setEnabled(true);


        this->save_all_BMP->setEnabled(true);
        this->save_all_TXT->setEnabled(true);

        this->AbortButton->setEnabled(false);

        this->StartButton->setEnabled(this->configurate);

        this->saveThread->set_stop(true);
        this->save_BMP_Thread->set_stop(true);

        this->bin2x2->setEnabled(true);
        this->pix_10b->setEnabled(true);
        this->pix_12b->setEnabled(true);

       // specific a cinema
          this->pix_8b->setEnabled(true);
          this->imx_nb_chan->setEnabled(true);
          this->imx_freq_div->setEnabled(false); // true

        this->exposure_doubleSpinBox->setEnabled(true);
        this->offset_val->setEnabled(true);
        this->ADC_gain->setEnabled(true);
        this->binX_val->setEnabled(true);
        this->binY_val->setEnabled(true);
        this->test_mode->setEnabled(true);


        this->actionProgram->setEnabled(true);
        this->actionProgram_SPI->setEnabled(true);
        this->actionConfigure_CL->setEnabled(true);
        this->actionScript->setEnabled(true);
        this->actionRead_SENSOR_registers->setEnabled(true);
        this->test_mode_conf->setEnabled(true);
        this->test_mode->setEnabled(true);
        this->Nand_Bypass->setEnabled(true);

        // Disabled for preliminary version
          this->DDR_mode_disable->setDisabled(true);
          // this->pix_10b->setDisabled(true);
          // this->pix_12b->setDisabled(true);
          this->binX_val->setMaximum(8);
          this->binY_val->setMaximum(8);
          this->imx_freq_div->setDisabled(true);
          this->imx_nb_chan->setDisabled(true);

        if (this->Nand_Bypass->isChecked()==false && this->last_seq_enable == true)
            this->read_last_button->setEnabled(true); // true
        else
            this->read_last_button->setEnabled(false);

    }


    if (!this->saveThread->isRunning() && !this->save_BMP_Thread->isRunning())
        qDebug().noquote() << "END";
    else
        qDebug().noquote() << "Saving image files in progress...";

}

void MainWindow::Enable_IHM_param(bool config_ok)
{

    this->ADC_gain->setEnabled(config_ok);
    this->binY_val->setEnabled(config_ok);
    this->binX_val->setEnabled(config_ok);


    this->test_mode->setEnabled(config_ok);
    this->test_mode_conf->setEnabled(config_ok);
    this->offset_val->setEnabled(config_ok);
    this->exposure_doubleSpinBox->setEnabled(config_ok);
    this->nb_images_val->setEnabled(config_ok);
    this->time_interval_val->setEnabled(config_ok);
    this->nb_frames_coadded->setEnabled(config_ok);
    this->video_infinite->setEnabled(config_ok);
    this->rst_camera->setEnabled(config_ok);
    this->width->setEnabled(config_ok);
    this->height->setEnabled(config_ok);
    this->offset_x->setEnabled(config_ok);
    this->offset_y->setEnabled(config_ok);

    this->imx_freq_div->setEnabled(config_ok);
    this->imx_nb_chan->setEnabled(config_ok);
    this->DDR_mode_disable->setEnabled(false);
    this->Nand_Bypass->setEnabled(config_ok);

    this->pix_8b->setEnabled(config_ok);
    this->pix_10b->setEnabled(config_ok);
    this->pix_12b->setEnabled(config_ok);

}


void MainWindow::ChannelNotOpen(void)
{
    this->LabelStatusBar->setText("Channel not open");
    QMessageBox msgBox;
    msgBox.setText("Channel not open");
    msgBox.exec();

    on_AbortButton_clicked();
    this->StartButton->setEnabled(false);
}

void MainWindow::PrintStatusBar(QString status)
{
    this->statusBar->showMessage(status, 1000);
    // qDebug().noquote() << status;
}


void MainWindow::on_IMX_freq_div_valueChanged()
{
    this->PrintStatusBar("Update sensor clk freq/calibrate");

    int cmv_freq_div = this->imx_freq_div->currentIndex();
    QString imx_string = this->imx_freq_div->currentText();

    this->compute_maximum_width();

    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);



   this->cam_controller->clk_config(this->cam_fw_type, static_cast<quint8>(cmv_freq_div), 0);
   this->compute_FPS();

   this->calib();
}


void MainWindow::on_CMV_freq_div_valueChanged(int cmv_freq_div)
{
    this->PrintStatusBar("Update sensor clk freq/calibrate");
    double cmv_in_freq = this->cam_controller->get_cmv_in_freq(this->cam_fw_type,static_cast<quint8>(cmv_freq_div));
    this->cmv_in_freq_lbl->setText("CMV_in_freq = " + QString::number(cmv_in_freq) + " MHz");

    this->compute_maximum_width();

    this->compute_FPS();

    this->cam_controller->clk_config(this->cam_fw_type, static_cast<quint8>(cmv_freq_div), 0);

    this->calib();
}


void MainWindow::on_IMX_nb_chan_valueChanged() {

//    qDebug() << "current index : " + QString::number(this->imx_nb_chan->currentIndex());
    this->nb_chan = static_cast<quint8>((pow(2,2+(this->imx_nb_chan->currentIndex()))));
    this->PrintStatusBar("Update number of channels to " + QString::number(nb_chan));

    this->compute_maximum_width();
    this->compute_FPS();

    double exposure_ms_MAX = this->imx->Get_exposure_MAX(quint8(this->imx_freq_div->currentIndex()), quint16(this->height->value()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());
    if (this->xtrig_en == false)
        this->exposure_doubleSpinBox->setMaximum(exposure_ms_MAX);

    this->on_exposure_doubleSpinBox_valueChanged(this->exposure_doubleSpinBox->value());
    this->cam_controller->Set_nb_chan_reg(this->nb_chan);
    this->imx->config( this->nb_chan,this->nb_bits_sensor, this->DDR_mode_disable->isChecked(), this->xtrig_en);
}


void MainWindow::on_offset_val_valueChanged(int arg1)
{
    this->PrintStatusBar("Update offset value");

    this->imx->Set_Offset(static_cast<quint16>(arg1));

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // Move window to the center at the beginning
    if (this->move_to_center) {
        QScreen *screen = QGuiApplication::primaryScreen();
        int move_width  = screen->geometry().width()/2 - this->size().width()/2;
        int move_height = screen->geometry().height()/2 - this->size().height()/2;
        this->move(move_width, move_height);
        this->move_to_center = false;
    }
    QWidget::resizeEvent(event);
    if (!pixmap_img.isNull()) {
//        qDebug() << "Before setPixmap";
        this->screen_lbl->setPixmap(pixmap_img.scaled(scrollArea->width(),scrollArea->height(),Qt::KeepAspectRatio));
//        qDebug() << "After setPixmap";
    }
}

void MainWindow::UpdateLastConfiguration()
{

 this->last_seq_nb_bits_sensor =  this->nb_bits_sensor;
 this->last_seq_binY_val = static_cast<quint8>(this->binX_val->value());
 this->last_seq_nb_frames_coadded = static_cast<quint8>(this->nb_frames_coadded->value());
 this->last_seq_binX_val   = static_cast<quint8>(this->binY_val->value());
 this->last_seq_spw_nbbytes_per_pixel = static_cast<quint8>(this->spw_nbbytes_per_pixel);//4
 this->last_seq_height = static_cast<quint16>(this->height->value());
 this->last_seq_fps_divider = static_cast<quint8>(this->fps_divider->value());
 this->last_seq_nb_images_val = static_cast<quint8>(this->nb_images_val->value());
 this->last_seq_imx_h_period = this->imx->Get_H_period_IMX(this->imx_freq_div->currentIndex(),this->nb_bits_sensor,this->nb_chan,this->DDR_mode_disable->isChecked());
 this->last_seq_imx_freq = this->cam_controller->Get_IMX_Freq(this->imx_freq_div->currentIndex());
 this->last_seq_offset_val = static_cast<quint16>(this->offset_val->value());
 this->last_seq_num_sequence = this->num_sequence;
 this->last_seq_ADC_gain = static_cast<quint8>(this->ADC_gain->value());
 this->last_seq_inter_frame = this->time_interval_val->value();
 this->last_temperature     = this->temperature;


}

void MainWindow::on_CloseButton_clicked()
{
    this->gui_params.Cl_camera.CameraLink_close();
}


void MainWindow::on_autosave_isChecked()
{
    if (this->save_all_BMP->isChecked() || this->save_all_TXT->isChecked())
    {
        int Buffsize = 4*(this->width->value()/this->binX_val->value())*((this->height->value()/this->binY_val->value())+1);
        int MaxNbBuffer = int(NB_COPY_BUFFER-2.56*Buffsize/1000000); //
        if (MaxNbBuffer < 25)
            MaxNbBuffer =  25;
        if (MaxNbBuffer > NB_COPY_BUFFER)
           MaxNbBuffer =  NB_COPY_BUFFER;
        qDebug().noquote() << "Maximum image that can be stored: " + QString::number(MaxNbBuffer,10);
    }
}


