#ifndef CMV4000_H
#define CMV4000_H

#include <QString>

#include "prospect_regs.h"
#include "cameralink.h"
#include "cmv_registers.h"

////////////////////////////////////////
// CMV4000 registers
////////////////////////////////////////
const quint32 PLL_ENABLE_AD                = 113;
const quint32 PLL_BYPASS_AD                = 115;
const quint32 NB_FRAMES_REG0_AD            =  70;
const quint32 NB_FRAMES_REG1_AD            =  71;

const quint32 CHANNEL_EN_REG0_AD           =  80;
const quint32 CHANNEL_EN_REG1_AD           =  81;
const quint32 CHANNEL_EN_REG2_AD           =  82;

const quint32 VERSION_AD                   = 125;
const quint32 BLACK_REF_COLUMNS            = 121;

const quint32 CMV_CONF_REG_00              = 0;
const quint32 CMV_CONF_REG_01              = 1;
const quint32 CMV_CONF_REG_02              = 2;

const quint32 CMV_CONF_REG_35              = 35;
const quint32 CMV_CONF_REG_36              = 36;
const quint32 CMV_CONF_REG_37              = 37;
const quint32 CMV_CONF_REG_38              = 38;

const quint32 CMV_CONF_REG_39              = 39; // mono


const quint32 CMV_CONF_REG_40              = 40; // Image flipping
// Recommended registers settings
const quint32 CMV_CONF_REG_41              = 41; // Inte_sync Exp_dual Exp_ext (3 bits)  required value = 4
const quint32 CMV_CONF_REG_42              = 42; // Exp[7:0]
const quint32 CMV_CONF_REG_43              = 43; // Exp[15:8]
const quint32 CMV_CONF_REG_44              = 44; // Exp[23:16]

const quint32 CMV_CONF_REG_72              = 72; // Output mode = 0 : 16 outputs, 1:8, 2:4, 3:2
const quint32 CMV_CONF_REG_73              = 73; // FOT

const quint32 CMV_CONF_REG_77              = 77; // Col_calib ADC_calib (2 bits)         required value = 0
const quint32 CMV_CONF_REG_78              = 78;
const quint32 CMV_CONF_REG_79              = 79;

//// channels enable/disable = power control (see datasheet)
const quint32 CMV_CONF_REG_80              = 80; // Channel_En = dft : 255
const quint32 CMV_CONF_REG_81              = 81; //            = dft : 255
const quint32 CMV_CONF_REG_82              = 82; //            = dft : 3

const quint32 CMV_CONF_REG_84              = 84; // I_col (4 bits)                       required value = 4
const quint32 CMV_CONF_REG_85              = 85; // I_col_prech (4 bits)                 required value = 1
const quint32 CMV_CONF_REG_86              = 86; // I_adc (4 bits)                       required value = 14
const quint32 CMV_CONF_REG_87              = 87; // I_amp (4 bits)                       required value = 12
const quint32 CMV_CONF_REG_88              = 88; // Vtf_l1 (7 bits)                      required value = 64
const quint32 CMV_CONF_REG_91              = 91; // Vres_low (7 bits)                    required value = 64
const quint32 CMV_CONF_REG_94              = 94; // V_prech (7 bits)                     required value = 101
const quint32 CMV_CONF_REG_95              = 95; // V_ref (7 bits)                       required value = 106

//V_ramp1/V_ramp2
const quint32 CMV_CONF_REG_98              = 98;
const quint32 CMV_CONF_REG_99              = 99;
//Offset
const quint32 CMV_CONF_REG_100             = 100;
const quint32 CMV_CONF_REG_101             = 101;

const quint32 CMV_CONF_REG_102             = 102; //

//ADC Reg
const quint32 CMV_CONF_REG_103             = 103;

const quint32 CMV_CONF_REG_111             = 111; //
const quint32 CMV_CONF_REG_112             = 112; //
const quint32 CMV_CONF_REG_113             = 113; //
const quint32 CMV_CONF_REG_114             = 114; //
const quint32 CMV_CONF_REG_115             = 115; //
const quint32 CMV_CONF_REG_116             = 116; //
const quint32 CMV_CONF_REG_117             = 117; //
const quint32 CMV_CONF_REG_118             = 118; //
const quint32 CMV_CONF_REG_121             = 121; //
const quint32 CMV_CONF_REG_123             = 123; //

const quint32 CMV_CONF_REG_126             = 126; // temperature
const quint32 CMV_CONF_REG_127             = 127; // temperature

class CMV4000
{
public:
    CMV4000(CameraLink *Cl_Camera, quint32 base_ad);

    quint16 read_temp_reg(void);
    void config(quint8 cam_fw_type, quint8 nb_chan      , // Number of data channels
                        quint8 bypass       , // 1 to bypass the internal PLL
                        quint8 nb_bits);      // 8, 10 or 12

    void Set_Offset(quint16 offset);
    quint16 Get_Offset();
    void Set_Vramp(quint8 value); // One value => both registers should be set at the same
    quint8 Get_Vramp1();
    quint8 Get_Vramp2();
    quint8 Get_ADC();
    void Set_ADC(quint8 value);
    void Set_PGA_gain(double gain, quint8 amplify);
    quint8 Get_FOT();
    bool Set_FOT(quint8 value);
    bool setTrainingPattern(quint16 training_pattern);

    quint8 Get_SensorType();

    void pll_config(double freq_in,
                    quint8 bypass); // 1 to bypass the internal PLL

    void mode_config(quint8 bit_mode); // Shall be 10 or 12

    void Channels_en(quint8 Reg80,  // Reg 80
                             quint8 Reg81,  // Reg 81
                             quint8 Reg82); // Reg 82

    quint32 exposure_f_to_int(double exposure_time, double cmv_in_freq, quint8 fot_length);
    QByteArray get_CMV_regs();
    void set_CMV_regs(QByteArray reg_values);

private:
    Cl_Camera *Cl_Camera;
    quint32 base_ad;
};

#endif // CMV4000_H
