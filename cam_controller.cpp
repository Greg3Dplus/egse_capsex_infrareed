#include "cam_controller.h"

CAM_controller::CAM_controller(IMX *imx, CameraLink *Cl_camera)
{
    this->Cl_camera = Cl_camera;
    this->imx = imx;

    this->Cl_camera->InitIcd();//charge 3DCM800.icd into 3DCM800_tem.icd
}


quint8 CAM_controller::config_SENS_CTL(quint8 cam_fw_type, quint8 test_mode, quint8 nb_bits, quint8 footer)
{

    // Configure the CMV_Conf_Reg register for binning mode, test mode, etc.
    int mode_int    = 1;     // 10 bits
    if (nb_bits == 12)
        mode_int = 0;
    else if (nb_bits == 8)
        mode_int = 3;
    else
        mode_int = 1;




        quint8 cmv_conf_val = static_cast<quint8>((mode_int       << 2)
                                                  );

        this->Cl_camera->send_TC(SENS_CTL_ba | CMVCTL_CONF_AD, cmv_conf_val);



    return 1;
}

double CAM_controller::get_cmv_in_freq(quint8 cam_fw_type, quint8 cmv_freq_div) {
//    if (cam_fw_type == CAM_FW_CO2M){  // n'est pas utilisée par l'IMX
//        return 192.0 / (2*(cmv_freq_div+1));
//    }
//    else {
        return 64.0 / (cmv_freq_div+1);
//    }
}

void CAM_controller::clk_config(quint8 cam_fw_type, quint8 cmv_freq_div, quint8 bypass) {


    this->Cl_camera->send_TC(SENS_CTL_ba | FREQ_DIV, cmv_freq_div);

    this->imx->pll_config(cmv_freq_div);

    // Wait until PLL is stable
    Sleep(10);
}


void CAM_controller::Set_Exp_imx(double exposure_time_ms, quint8 imx_in_freq,  quint8 nb_bits_sensor, quint8 nb_chan, quint32 height, quint8 DDR_mode_disable)
{
 
 double H_Period = this->imx->Get_H_period_IMX(imx_in_freq,  nb_bits_sensor, nb_chan, DDR_mode_disable);

// qDebug () << "H_Period         = " + QString::number(H_Period);
// qDebug () << "exposure_time    = " + QString::number(exposure_time_ms) + " ms";

 // in us
 quint32 SHS_int = quint32((height+VB1[1]+frameInfo[1]) - (((exposure_time_ms*1000) - (14.6)) / H_Period))+1;
// qDebug () << "SHS_int = 0x" + QString::number(SHS_int,16);

// for xtrig.
 double imx_in_freq_MHz;
 if (imx_in_freq == 0)
     imx_in_freq_MHz = 74.25;
 else if (imx_in_freq == 1)
     imx_in_freq_MHz = 54;
 else
     imx_in_freq_MHz = 37.125;


 quint32 Nb_tick_Exp_Xtrig = quint32((imx_in_freq_MHz*1000000)*exposure_time_ms/1000);
 this->Cl_camera->send_TC(4, SENS_CTL_ba | TRIG_EXP_TIME_AD0, Nb_tick_Exp_Xtrig);

// quint32 Nb_tick_interframe_Xtrig = quint32((imx_in_freq_MHz*1000000)*250/1000);  //50 ms interframe
// this->Cl_camera->send_TC(4, SENS_CTL_ba | INTER_IMAGE_AD0, Nb_tick_interframe_Xtrig);
// qDebug().noquote() << "NB Tick Exp XTRIG "+ QString::number(Nb_tick_Exp_Xtrig,10);

 this->imx->Set_SHS(SHS_int);
 //this->imx->Set_SHS(20);

}


double CAM_controller::Get_IMX_Freq(quint8 IMXFreqCurrentIndex)
{

    if (IMXFreqCurrentIndex==0)
        return 74.25;
    else if (IMXFreqCurrentIndex==1)
       return 54;
    else
      return  37.125;

}

void CAM_controller::Set_nb_chan_reg(quint8 nb_chan)
{

    quint8 value;
    if (nb_chan==16)
        value = 0;
    else if (nb_chan==8)
        value = 1;
    else
        value = 2;
//    qDebug ().noquote() << "Set register nb channel to  " + QString::number(value);
    this->Cl_camera->send_TC(1, SENS_CTL_ba | SENSCTL_NUM_CHAN_AD, value);
}

void CAM_controller::reset_sensor()
{
    qDebug ().noquote() << "Reset sensor";
    this->Cl_camera->send_TC(1, SENS_CTL_ba | CMVCTL_CTL_AD, 1 << 5);
    this->Cl_camera->send_TC(1, 0x187, 1);
}
