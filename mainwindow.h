#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include <QString>
#include <QLabel>
#include <QPixmap>
#include <QGridLayout>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QFileDialog>
#include <QMutex>
#include <QGroupBox>
#include <QCheckBox>
#include <QStatusBar>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QRadioButton>
#include <QScreen>
#include <QGuiApplication>
#include <QComboBox>

#include "streamingthreads.h"
#include "cu_frame.h"
#include "cmv_read_registers.h"
#include "imx_read_registers.h"
#include "test_mode_configuration.h"
#include "programwindow.h"
#include "sendcmddialog.h"
#include "CLdialog.h"
#include "init_file.h"
#include "saveimagethread.h"
#include "saveBMPthread.h"
#include "script.h"
#include "aboutwindow.h"
#include "cameralink.h"
//#include "spw_camera.h"
//#include "cmv4000.h"
#include "imx.h"
#include "mcfmc_registers.h"
#include "cam_controller.h"


#include "Windows.h"//Sleep for MSVC

#include "customspinbox.h"

const quint32 egse_version = 2;

const bool PRELIMINARY_VER = false;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, bool batch_mode = false);
    void resizeEvent(QResizeEvent *event);
    ~MainWindow();

    void streaming();

    void NOT_Configured();
    void updateAllParameters(void);



    void closeEvent (QCloseEvent *event);

    void req_bmp_file();
    void req_txt_file();


    void UpdateButton(void);
    void TOReceiveSPW(void);
    void SetError(void);
    void UpdateSensor(void);
    void CU_ConfigError(void);
    void EndThread(void);
    void EndSequence(void);
    void ChannelNotOpen(void);
    void Quit(void);
    void compute_FPS(void);
    quint8 get_CMV_freq_div(void);
    quint8 config_Binning(quint8 binning_X, quint8 binning_Y, quint8 bin_mode);

    bool calib(void);

    void  Abort_Video(void);
    void run_script(QString script_name, bool batch_mode);

    void SendVidReq(void);
    void compute_maximum_width();

private slots:
    void on_StartButton_clicked();
    void show_TM();
    void PrintStatusBar(QString status);

    void transmit_TC();
    void on_CMV_freq_div_valueChanged(int cmv_freq_div);
    void on_IMX_freq_div_valueChanged();

    void on_offset_val_valueChanged(int arg1);

    void on_binY_val_valueChanged();

    void on_binX_val_valueChanged();
    void on_bin_mode_val_valueChanged();
    void on_nb_frames_coadded_val_valueChanged(int value);
    void on_IMX_nb_chan_valueChanged();
    void on_test_configuration_clicked();
    void on_pix_8b_clicked();
    void on_pix_10b_clicked();
    void on_pix_12b_clicked();
   // void on_bin2x2_stateChanged();
    void on_DDR_mode_stateChanged();

    void on_test_mode_stateChanged();

    void on_width_valueChanged(int width);

    void on_height_valueChanged(int arg1);

    void on_offset_x_valueChanged(int arg1);

    void on_offset_y_valueChanged(int arg1);

    void on_ADC_gain_valueChanged(int arg1);
    void on_rst_camera_clicked();

    void on_save_TXT_pb_clicked();

    void on_ConfigButton_clicked();

    void on_save_BMP_pb_clicked();

    void on_actionProgram_triggered();
    void on_actionProgram_SPI_triggered();

    void on_actionScript_triggered(void);
    void on_action_exportScript_triggered(void);

    void on_actionExit_triggered();
    void on_actionAbout_triggered();

    void on_actionRead_SENS_registers_triggered();

    void on_actionSend_TC_TM_triggered();

    void on_actionConfigure_CL_triggered();

    void on_nb_images_val_valueChanged(int arg1);

    void on_video_infinite_stateChanged();

    void on_time_interval_val_valueChanged(double arg1);
    void on_fps_divider_valueChanged(int fps_divider);

    void on_AbortButton_clicked();

    void on_exposure_doubleSpinBox_valueChanged(double exposure_time_ms);

    void SendReadLast();

    void on_NAND_bypass();

    void UpdateLastConfiguration();

    void on_Read_last_sequence_clicked();

    void on_CloseButton_clicked();

    void Enable_IHM_param(bool config_ok);

    void on_autosave_isChecked();

private:
    bool stream;
    quint8 cam_hw_type;
    quint8 cam_fw_type;
    quint8 SensorIsColor;
    quint8 spw_txdiv_cnt;
    quint8 spw_nbbytes_per_pixel;
    double SysClk_freq;
    double SpwClk_freq;
    quint8 nb_bits_sensor;
    quint8 nb_chan;
    bool   configurate;
    qint32 max_width;
    qint32 max_height;
    quint8 fmc_bypass;
    bool xtrig_en;

    quint8 Cl_Config;
    quint8 TapNumber;

    CU_Frame *rcv_data_Byte;
    QPixmap pixmap_img;
    GUI_params gui_params;
    StreamingThreads *strThread;
    IMX  *imx;
    quint8 sensor;
    CAM_controller *cam_controller;

    sendcmddialog * window_send_TMTC;
    TMTC_params TM_params;
    TMTC_params TC_params;

    QByteArray TM_regs;
    bool pkt_received;
    QMutex mutex;

    quint32 num_sequence;
    quint32 num_image_bmp;

    init_file *init;
    QLabel *LabelStatusBar;

    saveImageThread *saveThread;
    saveBMPThread *save_BMP_Thread;

    QWidget* fenetre;
    QGridLayout *mainLayout;
    QStatusBar *statusBar;
    QWidget* first_column_widget;
    QGridLayout *first_column;
    QWidget* last_column_widget;
    QGridLayout *last_column;

    // Menu File
    QAction *actionRead_SENSOR_registers;
    QAction *actionSend_TC_TM;
    QAction *actionScript;
    QAction *action_exportScript;
    QAction *actionProgram;
    QAction *actionProgram_SPI;
    QAction *actionExit;
    QAction *actionConfigure_CL;

    // Menu About
    QAction *actionAbout;

    // 1st group box
    QGroupBox *cmv_params;
    QGridLayout *cmv_params_layout;

    QSpinBox *ADC_gain;
    CustomSpinBox *binX_val;
    QSpinBox *binY_val;
    QComboBox *bin_mode_val;
    QSpinBox *offset_val;
    QDoubleSpinBox *exposure_doubleSpinBox;
    QCheckBox *test_mode;
    QCheckBox *bin2x2;
    QPushButton *rst_camera;
    QRadioButton *pix_8b;
    QRadioButton *pix_10b;
    QRadioButton *pix_12b;

    // 2nd group box
    QGroupBox *video_settings;
    QGridLayout *video_params_layout;

    QSpinBox       *nb_images_val;
    QDoubleSpinBox *time_interval_val;
    QSpinBox       *fps_divider;
    QCheckBox      *video_infinite;
    QSpinBox       *nb_frames_coadded;

    QLabel *FPS;
    QLabel *max_FPS;

    // 3rd group box
    QGroupBox   *window_dim;
    QGridLayout *window_params_layout;
    QSpinBox    *width;
    QSpinBox    *height;
    QSpinBox    *offset_x;
    QSpinBox    *offset_y;

    // 4th group box
    QGroupBox   *freq_params;
    QGridLayout *freq_params_layout;

    QComboBox *imx_freq_div;
    QSpinBox  *CMV_freq_div;
    QSpinBox  *LVDS_nb_chan;
    QComboBox *imx_nb_chan;
    QCheckBox *DDR_mode_disable;

    QLabel *cmv_in_freq_lbl;
    QLabel *DDR_mode_lbl;
    QLabel *imx_in_freq_lbl;
    QLabel *pixel_rate_lbl;

    QPushButton *ConfigButton;
    QPushButton *StartButton;
    QPushButton *AbortButton;
    QPushButton *SPW_running;
    QPushButton *Error;
    QPushButton *test_mode_conf;
    QPushButton *CloseButton;

    QLabel *screen_lbl;
    QScrollArea *scrollArea;

    // 1st Group Box, Right column
    QGroupBox   *real_time_params;
    QGridLayout *real_time_params_layout;

    QLabel *temperature_label;

    // 2nd Group Box
    QGroupBox   *status_params;
    QGridLayout *status_params_layout;

    QLabel *test_label;
    QLabel *file_status;
    QLabel *valid_hardware;

    QPushButton *save_TXT_pb;
    QPushButton *save_BMP_pb;

    QCheckBox *save_all_TXT;
    QCheckBox *save_all_BMP;

    // NAND FLASH
    QGroupBox   *Nand_flash_group;
    QGridLayout *Nand_flash_layout;
    QPushButton *read_last_button;
    QCheckBox   *Nand_Bypass;
    // last sequence read
    quint8 last_seq_nb_bits_sensor;
    quint8 last_seq_binY_val;
    quint8 last_seq_nb_frames_coadded;
    quint8 last_seq_binX_val;
    quint8 last_seq_spw_nbbytes_per_pixel;
    quint16 last_seq_height;
    quint8 last_seq_fps_divider;
    quint8 last_seq_nb_images_val;
    double last_seq_imx_freq;
    double last_seq_imx_h_period;
    quint16 last_seq_offset_val;
    quint32 last_seq_num_sequence;
    quint8 last_seq_ADC_gain;
    bool last_seq_enable = false;
    int last_seq_inter_frame;
    double last_temperature;

    double temperature;

    script *scriptWindow;

    bool move_to_center;







};

#endif // MAINWINDOW_H
