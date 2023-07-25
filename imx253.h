#ifndef IMX253_H
#define IMX253_H

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

const quint32 IMX_VERSION_LSB               = 0x348;
const quint32 IMX_VERSION_MSB               = 0x349;
//const quint32 BLACK_REF_COLUMNS            = 121;


// CHIP 2
const quint32 IMX_CONF_STANDBY       	   = 0x200;
const quint32 IMX_CONF_STBLVDS             = 0X205;

const quint32 IMX_XMSTA                    = 0x20A;
const quint32 IMX_TRIGEN                   = 0x20B;
const quint32 IMX_CONF_ADBIT               = 0x20C;

const quint32 IMX_CONF_VMAX_LSB            = 0x210;
const quint32 IMX_CONF_VMAX_MID            = 0x211;
const quint32 IMX_CONF_VMAX_MSB            = 0x212;

const quint32 IMX_CONF_HMAX_LSB            = 0x214;
const quint32 IMX_CONF_HMAX_MSB            = 0x215;

const quint32 IMX_CONF_ODBIT               = 0x216;

const quint32 IMX_CONF_FREQ                = 0x21B;
const quint32 IMX_CONF_OPORTSEL            = 0x21C;
const quint32 IMX_CONF_TMOUT               = 0x21D;

const quint32 SENS_IMX_TEMP                = 0x21E;

const quint32 IMX_CONF_TOUTxSEL            = 0x226;
const quint32 IMX_CONF_TRIG_TOUTxSEL       = 0x229;

const quint32 SYNCSEL_AD                   = 0x236;

const quint32 IMX_CONF_PULSE1_CONF         = 0x26D;

const quint32 IMX_CONF_PULSE1_UP           = 0x270;
const quint32 IMX_CONF_PULSE1_DN           = 0x274;
const quint32 IMX_CONF_PULSE2_CONF         = 0x279;
const quint32 IMX_CONF_PULSE2_UP           = 0x27C;
const quint32 IMX_CONF_PULSE2_DN           = 0x280;

const quint32 IMX_CONF_INCKSEL0            = 0x289;
const quint32 IMX_CONF_INCKSEL1            = 0x28A;
const quint32 IMX_CONF_INCKSEL2            = 0x28B;
const quint32 IMX_CONF_INCKSEL3            = 0x28C;

const quint32 IMX_CONF_SHS_EXP_TIME        = 0x28D;

const quint32 IMX_LOWLAGTRG                = 0x2AE;


//CHIP 4
const quint32 IMX_CONF_GAIN_LSB            = 0x404;
const quint32 IMX_CONF_GAIN_MSB            = 0x405;
const quint32 IMX_CONF_GAINDLY             = 0x412;
const quint32 IMX_CONF_TEST_EN             = 0x438;
const quint32 IMX_CONF_BLKLVL              = 0x454;

//CHIP 5 ROI
const quint32 IMX_ROI_ON_AD0                = 0x500; // ROI zone 1 à 4

const quint32 IMX_ROI_1_PH_LSB              = 0x510; // offset y horizontal byte LSB
const quint32 IMX_ROI_1_PH_MSB              = 0x511; // offset y horizontal byte MSB
const quint32 IMX_ROI_1_PV_LSB              = 0x512; // offset x vertical byte LSB
const quint32 IMX_ROI_1_PV_MSB              = 0x513; // offset x vertical byte MSB
const quint32 IMX_ROI_1_WH_LSB              = 0x514; // size y horizontal byte LS
const quint32 IMX_ROI_1_WH_MSB              = 0x515; // size y horizontal byte MS
const quint32 IMX_ROI_1_WV_LSB              = 0x516; // size x vertical byte LSB
const quint32 IMX_ROI_1_WV_MSB              = 0x517; // size x vertical byte MSB

class IMX253
{
public:
    IMX253(CameraLink *Cl_camera, quint32 base_ad);

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
};

#endif // CMV4000_H
