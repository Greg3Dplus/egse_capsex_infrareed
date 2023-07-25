#ifndef IMX_H
#define IMX_H

#include <QString>

#include "prospect_regs.h"
#include "cameralink.h"

#include "Windows.h"//Sleep for MSVC

////////////////////////////////////////
// CMV4000 registers
////////////////////////////////////////
//const quint32 PLL_ENABLE_AD                = 113;
//const quint32 PLL_BYPASS_AD                = 115;
//const quint32 NB_FRAMES_REG0_AD            =  70;
//const quint32 NB_FRAMES_REG1_AD            =  71;

//const quint32 CHANNEL_EN_REG0_AD           =  80;
//const quint32 CHANNEL_EN_REG1_AD           =  81;
//const quint32 CHANNEL_EN_REG2_AD           =  82;

const quint32 IMX_VERSION_LSB[2]            = {0x348,0xA16};
const quint32 IMX_VERSION_MSB[2]            = {0x349,0xA17};
//const quint32 BLACK_REF_COLUMNS            = 121;


// CHIP 2
const quint32 IMX_CONF_STANDBY[2]          = {0x200,0x200};
const quint32 IMX_CONF_STBLVDS[2]          = {0x205,0x644};

const quint32 IMX_XMSTA[2]                 = {0x20A,0x20C};
const quint32 IMX_TRIGEN[2]                = {0x20B,0x600};
const quint32 IMX_CONF_ADBIT[2]            = {0x20C,0x400};

const quint32 IMX_CONF_VMAX_LSB[2]            = {0x210,0x2D4};
const quint32 IMX_CONF_VMAX_MID[2]            = {0x211,0x2D5};
const quint32 IMX_CONF_VMAX_MSB[2]            = {0x212,0x2D6};

const quint32 IMX_CONF_HMAX_LSB[2]            = {0x214,0x2D8};
const quint32 IMX_CONF_HMAX_MSB[2]            = {0x215,0x2D9};

const quint32 IMX_CONF_ODBIT[2]               = {0x216,0x630};

const quint32 IMX_CONF_FREQ[2]                = {0x21B, 0x426};
const quint32 IMX_CONF_OPORTSEL[2]            = {0x21C,0x645};
const quint32 IMX_CONF_TMOUT[2]               = {0x21D,0x788};

const quint32 SENS_IMX_TEMP[2]                = {0x21E,0x690};


const quint32 IMX_CONF_TOUTxSEL[2]            = {0x226,0x635};
const quint32 IMX_CONF_TRIG_TOUTxSEL[2]       = {0x229,0x63A};

const quint32 SYNCSEL_AD[2]                   = {0x236,0x63C};

const quint32 IMX_CONF_PULSE1_CONF[2]         = {0x26D,0x678};
const quint32 IMX_CONF_PULSE1_UP  [2]         = {0x270,0x679};
const quint32 IMX_CONF_PULSE1_DN  [2]         = {0x274,0x67C};
const quint32 IMX_CONF_PULSE2_CONF[2]         = {0x279,0x680};
const quint32 IMX_CONF_PULSE2_UP  [2]         = {0x27C,0x681};
const quint32 IMX_CONF_PULSE2_DN  [2]         = {0x280,0x684};

const quint32 IMX_CONF_INCKSEL0[2]         = {0x289,0x420};
const quint32 IMX_CONF_INCKSEL1[2]         = {0x28A,0x421};
const quint32 IMX_CONF_INCKSEL2[2]         = {0x28B,0x424};
const quint32 IMX_CONF_INCKSEL3[2]         = {0x28C,0x425};

const quint32 IMX_CONF_SHS_EXP_TIME[2]     = {0x28D,0x440};

const quint32 IMX_LOWLAGTRG[2]             = {0x2AE,0x430};


//CHIP 4
const quint32 IMX_CONF_GAIN_LSB[2]            = {0x404,0x714};
const quint32 IMX_CONF_GAIN_MSB[2]            = {0x405,0x715};
const quint32 IMX_CONF_GAINDLY[2]             = {0x412,0x748};
const quint32 IMX_CONF_TEST_EN[2]             = {0x438,0x750};
const quint32 IMX_CONF_BLKLVL[2]              = {0x454,0x7C0};

//CHIP 5 ROI
const quint32 IMX_ROI_ON_AD0[2]           = {0x500,0x304}; // ROI zone 1 à 4
const quint32 IMX_ROI_1_PH_LSB[2]         = {0x510,0x320}; // offset y horizontal byte LSB
const quint32 IMX_ROI_1_PH_MSB[2]         = {0x511,0x321}; // offset y horizontal byte MSB
const quint32 IMX_ROI_1_PV_LSB[2]         = {0x512,0x322}; // offset x vertical byte LSB
const quint32 IMX_ROI_1_PV_MSB[2]         = {0x513,0x323}; // offset x vertical byte MSB
const quint32 IMX_ROI_1_WH_LSB[2]         = {0x514,0x324}; // size y horizontal byte LS
const quint32 IMX_ROI_1_WH_MSB[2]         = {0x515,0x325}; // size y horizontal byte MS
const quint32 IMX_ROI_1_WV_LSB[2]         = {0x516,0x326}; // size x vertical byte LSB
const quint32 IMX_ROI_1_WV_MSB[2]         = {0x517,0x327}; // size x vertical byte MSB

class IMX
{
public:
    IMX(CameraLink *Cl_camera, quint32 base_ad, quint8 cam_fw_type);

    void config(quint8 nb_chan, // Number of data channels
                        quint8 nb_bits, quint8 DDR_mode_disable, bool xtrig_en);      // 8, 10 or 12

    void Set_Offset(quint16 offset);
    quint16 Get_Offset();

    quint16 Get_ADC();
    void Set_VMAX(quint16 height);
    void Set_ADC(quint32 value);
    void Set_SHS(quint32 value);
    void Set_Offset_y(quint16 offset_y);
    void Set_PGA_gain(double gain, quint8 amplify);
    void setXTrigEnable(bool mode);
    quint8 Get_FOT();
    bool Set_FOT(quint8 value);
    double Get_FPS(quint32 height, double imx_in_freq, quint8 nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable);
    quint8 Get_SensorType();

    void pll_config(quint8 freq_in);

    void mode_config(quint8 bit_mode); // Shall be 8, 10 or 12

    quint32 exposure_f_to_int(double exposure_time, double IMX_in_freq, quint8 fot_length);
    QByteArray get_IMX_regs();
    void set_IMX_regs(QByteArray reg_values);
    QByteArray get_test_regs();
    bool Get_Test_Mode();
    void EnableTestMode(bool enable);
    void enable_PGMODE(bool enable);
    void set_test_regs(QByteArray reg_values);
    void configFREQ(quint8 DDR_mode_disable);
    quint16 GetHMAX(quint8 nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable);
    double Get_IMX_Bandwidth(quint8 nb_chan, quint8 nb_bits);

    double Get_temperature();

    double Get_exposure_MAX(quint8 imx_in_freq, quint16 height, quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable);
    double Get_H_period_IMX(quint8 imx_in_freq, quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable);

    void enable_standby_mode(bool enable);
    void disable_master_mode();

private:
    CameraLink *Cl_camera;
    quint32 base_ad;
    quint8  cam_type;
};

#endif // CMV4000_H
