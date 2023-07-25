#include "cmv4000.h"

CMV4000::CMV4000(SPW_camera *spw_camera, quint32 base_ad)
{
    this->spw_camera = spw_camera;
    this->base_ad = base_ad;
}

quint8 CMV4000::Get_SensorType()
{
    //qDebug() << "Check CMV VERSION...";
    quint8 cmv_version = this->spw_camera->send_TM(this->base_ad | VERSION_AD);

    if (cmv_version == 0x43) {
        return 4;
    }
    else {
        qDebug() << "No sensor found!!!!!! : cmv_version:" << cmv_version;
        return 0;
    }
}

void CMV4000::config(quint8 cam_fw_type,  quint8   nb_chan      , // Number of data channels
                     quint8   bypass       , // 1 to bypass the internal PLL
                     quint8   nb_bits)       // 8, 10 or 12
{

    // Print current configuration
    //qDebug() << "    Num of channels   : " + QString::number(nb_chan, 10);

    // REG 41 : Exposures register *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_41, 4);

    // REG 72 : output mode ( 2,4,8, or 16 channels) => set according to reg 114/116 (pll freq ..) frequency must be set according to the number of channels used, hence the pixel data readout frequency
    quint8 output_mode = 3;
    if    (nb_chan ==  4) {
        output_mode = 2;
    }
    else if (nb_chan ==  8) {
        output_mode = 1;
    }
    else if (nb_chan == 16) {
        output_mode = 0;
    }
    //qDebug() << "Set Output mode to " + QString::number(nb_chan, 10) + " channels (register to " + QString::number(output_mode, 10) + ")";
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_72, output_mode);

    //qDebug() << "Set Registers to required values...";

    // (*) : Value required => see Datasheet

    // REG 77 : Col_Calib/ADC_Calib *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_77, 0);

    // REG 84 : I_Col *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_84, 4);

    // REG 85 : I_col_prech *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_85, 1);

    // REG 86 : *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_86, 14);

    // REG 87 : I_amp *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_87, 12);

    // REG 88 : Vtf_l1 *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_88, 64);

    // REG 91 : VRes_low *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_91, 64);

    // REG 94 : V_prech *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_94, 101);

    // REG 95 : Vref *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_95, 106);

    // REG 102 : PGA_Gain *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_102, 1);

    // REG 118 : Dumint *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_118, 1);

    // REG 123 : V_black_sun *
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_123, 98);


    int chan_en = 0xFFFF;
    if (nb_chan ==  2) {
        chan_en = 0x0101;
    }
    else if (nb_chan ==  4) {
        chan_en = 0x1111;
    }
    else if (nb_chan ==  8) {
        chan_en = 0x5555;
    }
    else if (nb_chan == 16) {
        chan_en = 0xFFFF;
    }

    if (cam_fw_type == CAM_FW_3DIPCC0746 || cam_fw_type == CAM_FW_3DIPCC0802){
        // VHDL code always try to calibrate the 8 data channels
        chan_en = 0x5555;
    }

    quint8 Reg82_val = 0x03;
    if (bypass == 1) {
        Reg82_val = 0x07;
    }
    this->Channels_en( (chan_en >> 0) & 0xff,  // Reg 80
                       (chan_en >> 8) & 0xff,  // Reg 81
                       Reg82_val);             // Reg 82

    // Configure resolution of the sensor
    mode_config(nb_bits);    // 10 or 12
}

void CMV4000::pll_config(double freq_in, quint8 bypass)
{
    //qDebug() << "### Configure CMV PLL with freq_in = " + QString::number(freq_in) + " MHz, bypass = " + QString::number(bypass);

    quint8 pll_enable = 1;
    if (bypass == 1) {
        pll_enable = 0;
    }
    quint8 pll_bypass = bypass;

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_113, pll_enable);

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_115, pll_bypass);

    quint8 pll_range = 0;
    if (freq_in >= 30.0
            ||  (freq_in >= 15.0 && freq_in < 20.0)
            ||  (freq_in >=  7.5 && freq_in < 10.0)) {
        pll_range = 1;
    }

    quint8 pll_out_fre = 0;
    if    (freq_in >=  7.5 && freq_in < 15.0) {
        pll_out_fre = 2;
    }
    else if (freq_in >= 15.0 && freq_in < 30.0) {
        pll_out_fre = 1;
    }
    else if (freq_in >= 30.0) {
        pll_out_fre = 5;
    }

    quint8 pll_in_fre = 0;
    if    (freq_in >= 10.0 && freq_in < 20.0) {
        pll_in_fre = 1;
    }
    else if (freq_in < 10.0) {
        pll_in_fre = 3;
    }

    // PLL Configuration => ClkPix = 10* ClkIn
    // REG 114 => PLL_IN_FRE
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_114, pll_in_fre);

    quint8 pll_div = this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_116) & 0xf; // 4 bits
    // int pll_div = 0x9;  // 4 bits

    // REG 116 => PLL_range / PLL_OUT_FRE
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_116, static_cast<quint8>((pll_range   << 7) |
                                                                                    (pll_out_fre << 4) |
                                                                                    (pll_div     << 0)));
}

/*******************************************************/
void CMV4000::mode_config(quint8 bit_mode)
{
    quint8 bit_mode_int = 1;
    if (bit_mode == 12) {
        bit_mode_int = 0;
    }

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_111, bit_mode_int);

    quint8 adc_res = 0;
    if (bit_mode == 12) {
        adc_res = 2;
    }

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_112, adc_res);

    quint8 pll_load = 8;
    if (bit_mode == 12) {
        pll_load = 4;
    }

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_117, pll_load);

    quint8 reg_116 = this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_116); // 4 bits

    // Shall be set to  9 in 10 bits mode
    // Shall be set to 11 in 12 bits mode
    quint8 pll_div = 9;  // 4 bits
    if (bit_mode == 12) {
        pll_div = 11;
    }

    // REG 116 => PLL_range / PLL_OUT_FRE
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_116, static_cast<quint8>((reg_116 & 0xf0) |
                                                                                    (pll_div     << 0)));
}

void CMV4000::Channels_en(quint8 Reg80, quint8 Reg81, quint8 Reg82) {
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_80, Reg80);
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_81, Reg81);
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_82, Reg82);
}

quint16 CMV4000::read_temp_reg(void) {
    QByteArray temp_reg = this->spw_camera->send_TM(2, this->base_ad | CMV_CONF_REG_126);

    return static_cast<quint16>(((temp_reg.at(1)) << 8) | temp_reg.at(0));
}

void CMV4000::Set_PGA_gain(double gain, quint8 amplify) {

    quint8 value = 1;

    qDebug () << "gain = " + QString::number(gain);

    if (gain == 1) {
        value = 0;
    }
    else if (gain == 1.4){
        value = 2;
    }
    else if (gain == 1.6){
        value = 3;
    }
    else {
        value = 1;
    }

    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_102, value);
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_121, amplify);
}

void  CMV4000::Set_Offset(quint16 offset) {
    QByteArray data;
    data.append(static_cast<qint8>(offset));
    data.append(static_cast<qint8>(offset >> 8));
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_100, data);
}

quint16 CMV4000::Get_Offset() {
    QByteArray data = this->spw_camera->send_TM(2, this->base_ad | CMV_CONF_REG_100);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;
    return (data0 | data1);
}

quint8 CMV4000::Get_ADC() {
    return this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_103);
}

void  CMV4000::Set_Vramp(quint8 value) {
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_98, value);
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_99, value);
}

quint8 CMV4000::Get_FOT() {
    return this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_73);
}

quint8 CMV4000::Get_Vramp1() {
    return this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_98);
}

quint8 CMV4000::Get_Vramp2() {
    return this->spw_camera->send_TM(this->base_ad | CMV_CONF_REG_99);
}

void  CMV4000::Set_ADC(quint8 value) {
    this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_103, value);

    // Minimum ADC gain settling time is 20 ms for CMV4000
    Sleep(20);
}

bool CMV4000::Set_FOT(quint8 value) {
    return this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_73, value);
}

bool CMV4000::setTrainingPattern(quint16 training_pattern) {
    QByteArray data;
    data.clear();
    data.append(static_cast<qint8>(training_pattern & 0xff));
    data.append(static_cast<qint8>(training_pattern >> 8));

    //Set training pattern
    return this->spw_camera->send_TC(this->base_ad | CMV_CONF_REG_78, data);
}

QByteArray CMV4000::get_CMV_regs() {

    QByteArray cmv_regs_ar;

    //qDebug() << "send_TM 128";
    cmv_regs_ar = this->spw_camera->send_TM(128, this->base_ad | CMV_CONF_REG_00);

    return cmv_regs_ar;
}

void CMV4000::set_CMV_regs(QByteArray reg_values) {
    this->spw_camera->write_data(this->base_ad | CMV_CONF_REG_00, reg_values);
}

// Convert exposure time in us to quint32 register value as defined in CMV datasheet
quint32 CMV4000::exposure_f_to_int(double exposure_time, double cmv_in_freq, quint8 fot_length) {
    double clk_per = 1 / cmv_in_freq;

    //qDebug () << "cmv_in_freq = " + QString::number(cmv_in_freq) + "MHz";
    //qDebug () << "clk_per = " + QString::number(clk_per) + "us";

    qint32 exp_time_int = static_cast<qint32>((exposure_time / (129.0*clk_per) - 0.43 * fot_length));

    if (exp_time_int < 0) {
        return 0;
    }

    return static_cast<quint32>(exp_time_int);
}
