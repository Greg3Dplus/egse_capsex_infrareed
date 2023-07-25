#include "imx253.h"

IMX253::IMX253(CameraLink *Cl_camera, quint32 base_ad)
{
    this->Cl_camera = Cl_camera;
    this->base_ad = base_ad;
}

quint8 IMX253::Get_SensorType()
{

    this->enable_standby_mode(false);

    // Sleep(10);

    qDebug() << "Check IMX VERSION...";
    QByteArray IMX_version = this->Cl_camera->send_TM(2,this->base_ad | IMX_VERSION_LSB);
    quint8 sensor_type = 0;
     this->enable_standby_mode(true);

     if (IMX_version.at(0) == 0x40) {
         if (((IMX_version.at(1) & 0xff) == 0xbf) // LLR (monochrome)
          || ((IMX_version.at(1) & 0xff) == 0x3f)) { // LQR (color)

            if ((IMX_version.at(1) & 0xff) == 0x3f) {
              qDebug().noquote() << "IMX253 LQR (color version)";
            }
            else {
              qDebug().noquote() << "IMX253 LLR (monochrome version)";
            }
            sensor_type= 3;
        }
     }
     else {
         qDebug() << "No sensor found!!!!!! : IMX_version: 0x" + QString::number(IMX_version.at(0),16) + " and 0x" + QString::number(IMX_version.at(1),16);
         sensor_type= 0;
     }

 return sensor_type;

}

void IMX253::enable_standby_mode(bool enable)
{
//    if (enable == true) {
//        qDebug().noquote() << "ENABLE Standby mode";
//    }
//    else {
//        qDebug().noquote() << "DISABLE Standby mode";
//    }
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STANDBY,  quint8(enable));
}

void IMX253::enable_PGMODE(bool enable)
{
    quint8 PGMODE;
    if (enable == true) {
        qDebug().noquote() << "ENABLE Test pattern";
        PGMODE = 0x1D;
    }
    else {
        qDebug().noquote() << "DISABLE Test pattern";
        PGMODE = 0x6;
    }
    this->Cl_camera->send_TC(this->base_ad | 0x438,  PGMODE);
}

void IMX253::disable_master_mode()
{
    this->Cl_camera->send_TC(this->base_ad | IMX_XMSTA, 0x1 );
}


void IMX253::config(quint8   nb_chan, // Number of data channels
                    quint8   nb_bits, quint8 DDR_mode_disable, bool xtrig_en)       // 8, 10 or 12
{

    // quint32 value = 2986; // number of row 3000
    // SET VMAX
//    this->s->send_TC(this->base_ad | IMX_CONF_VMAX_LSB,  quint8 (value));
//    this->spw_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MID,  quint8 (value>>8));
//    this->spw_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MSB,  quint8 (value>>16));

//    qDebug().noquote() << "ATTENTION FAST TRIGGER MODE !!!!";
//    this->spw_camera->send_TC(this->base_ad | IMX_TRIGEN, 0x1);
//    this->spw_camera->send_TC(this->base_ad | IMX_LOWLAGTRG, 0x1);
//    this->spw_camera->send_TC(this->base_ad | IMX_XMSTA, 0x0);
//    qDebug().noquote() << "###############################################";

    //SET HMAX
    quint16 HMAX = GetHMAX(nb_chan,nb_bits,DDR_mode_disable);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_HMAX_LSB,  quint8 (HMAX));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_HMAX_MSB,  quint8 (HMAX>>8));

//    qDebug().noquote() << "HMAX = 0x" + QString::number(HMAX,16);

    //NB channel
    if (nb_chan==4){
       this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL, 0x30);
       this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS,  0x20);
    }
   else if(nb_chan==8){
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL, 0x10);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS,  0x10);
    }
    else {
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL, 0x90);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS,  0x00);
    }

    //TOUTxSEL
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TOUTxSEL,  0x0);
    //TRIG TOUTxSEL
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TRIG_TOUTxSEL,  0x0);

    //PULSE
    quint8 i;
    quint8 y;

    // this->spw_camera->send_TC(this->base_ad | IMX_CONF_PULSE1_CONF,  0x1);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE1_CONF,  0x0);

    for (i= 1; i<3;i++) {
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP)+i,  0x0);
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN)+i,  0x0);
    }
    // this->spw_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP)+0,  0x03);
    // this->spw_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN)+0,  0x60);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP)+0,  0x0);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN)+0,  0x0);

    // this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF,  0x05);
    //this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF,  0x09);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF,  0x00);
    for (y= 1; y<3;y++) {
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP)+y,  0x0);
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN)+y,  0x0);
    }

    // this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP)+0,  0x03);
    // this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN)+0,  0x05);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP)+0,  0x0);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN)+0,  0x0);

    Sleep(10);

    //qDebug().noquote() << "Set RECOMMENDED values";
    this->Cl_camera->send_TC(this->base_ad | 0x0365, 0x40);
    this->Cl_camera->send_TC(this->base_ad | 0x0366, 0x00);
    this->Cl_camera->send_TC(this->base_ad | 0x0382, 0x01);
    this->Cl_camera->send_TC(this->base_ad | 0x0426, 0x03);

    this->Cl_camera->send_TC(this->base_ad | 0x0474, 0x10);  // VOPB_VBLK_HWIDTH
    this->Cl_camera->send_TC(this->base_ad | 0x0475, 0x10);  // VOPB_VBLK_HWIDTH
    this->Cl_camera->send_TC(this->base_ad | 0x0476, 0x10);  // FINFO_HWIDTH
    this->Cl_camera->send_TC(this->base_ad | 0x0477, 0x10);  // FINFO_HWIDTH

    this->Cl_camera->send_TC(this->base_ad | 0x0718, 0x78);
    this->Cl_camera->send_TC(this->base_ad | 0x0719, 0x0C);
    this->Cl_camera->send_TC(this->base_ad | 0x0F70, 0x1E);
    this->Cl_camera->send_TC(this->base_ad | 0x0F71, 0x00);
    this->Cl_camera->send_TC(this->base_ad | 0x0F72, 0x67);
    this->Cl_camera->send_TC(this->base_ad | 0x0F73, 0x01);
    this->Cl_camera->send_TC(this->base_ad | 0x0F74, 0x1E);
    this->Cl_camera->send_TC(this->base_ad | 0x0F75, 0x00);
    this->Cl_camera->send_TC(this->base_ad | 0x0F76, 0x67);
    this->Cl_camera->send_TC(this->base_ad | 0x0F77, 0x01);
    this->Cl_camera->send_TC(this->base_ad | 0x1202, 0x20);
    this->Cl_camera->send_TC(this->base_ad | 0x1217, 0x03);
    this->Cl_camera->send_TC(this->base_ad | 0x121E, 0x03);
    this->Cl_camera->send_TC(this->base_ad | 0x123D, 0x24);
    this->Cl_camera->send_TC(this->base_ad | 0x1240, 0x09);
    this->Cl_camera->send_TC(this->base_ad | 0x1241, 0x6A);
    this->Cl_camera->send_TC(this->base_ad | 0x124A, 0xC0);
    this->Cl_camera->send_TC(this->base_ad | 0x1294, 0x06);



    // Configure resolution of the sensor
    setXTrigEnable(xtrig_en); // false SHS, true XTRIG
    mode_config(nb_bits);    // 8 or 10 or 12
    configFREQ(DDR_mode_disable);


    // Sleep(1000);

}

void IMX253::pll_config(quint8 freq_in)
{

    if (freq_in == 0){ // 74.25
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0,  16);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1,  0);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2,  16);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3,  0);
    }else if (freq_in == 1){ //54
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0,  22);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1,  0);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2,  22);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3,  0);
    }else { //37.125
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0,  16);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1,  2);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2,  16);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3,  2);
    }

}

///*******************************************************/
void IMX253::mode_config(quint8 bit_mode)
{
    //ADBBIT
    if (bit_mode == 8){
         this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT,  2);
         this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT,  2);

    }else if(bit_mode == 12){
         this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT,  1);
         this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT,  1);
    }else {
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT,  0);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT,  0);
    }
}

void IMX253::setXTrigEnable(bool mode){

    if (mode==true){
//    qDebug().noquote() << "ATTENTION FAST TRIGGER MODE !!!!";
    this->Cl_camera->send_TC(this->base_ad | IMX_TRIGEN, 0x1);
    this->Cl_camera->send_TC(this->base_ad | IMX_LOWLAGTRG, 0x1);
//    qDebug().noquote() << "###############################################";
    }
    else{

        this->Cl_camera->send_TC(this->base_ad | IMX_TRIGEN, 0x0);
        this->Cl_camera->send_TC(this->base_ad | IMX_LOWLAGTRG, 0x0);
    }
}

double IMX253::Get_temperature() {
    enable_standby_mode(false);
    // TMD_LATCH (Update value of TMOUT)
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TMOUT ,  0x1);

    QByteArray data = this->Cl_camera->send_TM(2, this->base_ad | SENS_IMX_TEMP);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;

    // TMD_LATCH (Set to 0 before next reading)
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TMOUT,  0x0);

    quint16 temp_val = (data0 | data1);

    double temp_celsius = 246.312 - 0.304 * temp_val;
//    qDebug().noquote() << "Temp °C = "+ QString::number(temp_celsius);
    enable_standby_mode(true);
    return temp_celsius;
}

void  IMX253::Set_Offset(quint16 offset) {
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_BLKLVL  ,  quint8(offset));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_BLKLVL+1,  quint8(offset>>8));
}

quint16 IMX253::Get_Offset() {
    QByteArray data = this->Cl_camera->send_TM(2, this->base_ad | IMX_CONF_BLKLVL);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;
    return (data0 | data1);
}

bool IMX253::Get_Test_Mode(){

    QByteArray data =  this->Cl_camera->send_TM(1, this->base_ad | IMX_CONF_TEST_EN);

    if (data.at(0) % 2 == 1) // bit LSB à 1 => test mode enable
        return true;
    else
        return false;
}

void IMX253::EnableTestMode(bool enable){

    QByteArray data =  this->Cl_camera->send_TM(1, this->base_ad | IMX_CONF_TEST_EN);

    if (enable == true)
        this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN, ( quint8(data.at(0)) | 0x01) );
    else
        this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN, ( quint8(data.at(0)) & 0xFE) );

}

quint16 IMX253::Get_ADC() {
    QByteArray data = this->Cl_camera->send_TM(2,this->base_ad | IMX_CONF_GAIN_LSB);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;
return (data0 | data1);
}

//void  IMX253::Set_Vramp(quint32 value) {
//}
void  IMX253::Set_SHS(quint32 value) {

//    quint8 SHS[3];
//    SHS[0] =(value >>  0) & 0xff;
//    SHS[1] =(value >>  8) & 0xff;
//    SHS[2] =(value >> 16) & 0xff;
//    qDebug().noquote() << "Set SHS to 0x" + QString::number(value,16) + ", ("+QString::number(value,10) + ")";
    this->Cl_camera->send_TC(3,this->base_ad |  IMX_CONF_SHS_EXP_TIME, value);
}

void  IMX253::Set_ADC(quint32 value) {

//    qDebug().noquote() << "Set GAIN to 0x" + QString::number(value,16) + "starting at address 0x" + QString::number(this->base_ad | IMX_CONF_GAIN_LSB,16);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_GAIN_LSB, quint8 (value));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_GAIN_MSB, quint8 (value>>8));
}

void IMX253::Set_VMAX(quint16 height){

    quint32 value = height + VB1 + frameInfo;
//    qDebug().noquote() << "Set VMAX to " +QString::number(value,10);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_LSB,  quint8 (value));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MID,  quint8 (value>>8));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MSB,  quint8 (value>>16));

    //set ROI enable
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_ON_AD0,  3);
    //Set ROI size height
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WV_LSB,  quint8 (height));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WV_MSB,  quint8 (height>>8));
    //Set ROI size width
    quint16 width = 4112;
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WH_LSB,  quint8 (width));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WH_MSB,  quint8 (width>>8));


}

void IMX253::Set_Offset_y(quint16 offset_y)
{

    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_PV_LSB,  quint8 (offset_y));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_PV_MSB,  quint8 (offset_y>>8));

}

//bool IMX253::setTrainingPattern(quint16 training_pattern) {
//    QByteArray data;
//    data.clear();
//    data.append(static_cast<qint8>(training_pattern & 0xff));
//    data.append(static_cast<qint8>(training_pattern >> 8));

//    //Set training pattern
//    //return this->Cl_camera->send_TC(this->base_ad | IMX_CONF_REG_78, data);
//    return 0;
//}

QByteArray IMX253::get_IMX_regs() {

    QByteArray IMX_regs_ar;

    // qDebug() << "send_TM 128";
    // IMX_regs_ar = this->Cl_camera->send_TM(128, this->base_ad | IMX_CONF_REG_00);

    int address_base_imx[118]   = {0x200,0x205,0x208,0x20A,0x20B,0X20C,0x20D,0x20E,0x210,0x211,0x212,0x214,0x215,0x216,0x21B,0x21C,
                          0x226,0x229,0x236,0x26D,0x270,0X271,0x272,0x274,0x275,0x276,0x279,0x27C,0x27D,0x27E,0x280,0x281,
                          0x282,0x289,0x28A,0x28B,0x28C,0x28D,0x28E,0x28F,0x2AA,0x2AE,0x2B0,0x404,0x405,0x412,0x454,0x455,
                          0x474,0x475,0x476,0x477,0x500,0x501,0x510,0x511,0x512,0x513,0x514,0x515,0x516,0x517,0x518,0x519,
                          0x51A,0x51B,0x51C,0x51D,0x51E,0x51F,0x520,0x521,0x522,0x523,0x524,0x525,0x526,0x527,0x528,0x529,
                          0x52A,0x52B,0x52C,0x52D,0x52E,0x52F,0x530,0x531,0x532,0x533,0x534,0x535,0x536,0x537,0x538,0x539,
                          0x53A,0x53B,0x53C,0x53D,0x53E,0x53F,0x540,0x541,0x542,0x543,0x544,0x545,0x546,0x547,0x548,0x549,
                          0x54A,0x54B,0x54C,0x54D,0x54E,0x54F};

    for(int i=0; i < 118; i++) {
      IMX_regs_ar[i]= this->Cl_camera->send_TM(this->base_ad | address_base_imx[i]);

    }

    return IMX_regs_ar;
}

void IMX253::set_IMX_regs(QByteArray reg_values) {

    int address_base_imx[118]   = {0x200,0x205,0x208,0x20A,0x20B,0X20C,0x20D,0x20E,0x210,0x211,0x212,0x214,0x215,0x216,0x21B,0x21C,
                          0x226,0x229,0x236,0x26D,0x270,0X271,0x272,0x274,0x275,0x276,0x279,0x27C,0x27D,0x27E,0x280,0x281,
                          0x282,0x289,0x28A,0x28B,0x28C,0x28D,0x28E,0x28F,0x2AA,0x2AE,0x2B0,0x404,0x405,0x412,0x454,0x455,
                          0x474,0x475,0x476,0x477,0x500,0x501,0x510,0x511,0x512,0x513,0x514,0x515,0x516,0x517,0x518,0x519,
                          0x51A,0x51B,0x51C,0x51D,0x51E,0x51F,0x520,0x521,0x522,0x523,0x524,0x525,0x526,0x527,0x528,0x529,
                          0x52A,0x52B,0x52C,0x52D,0x52E,0x52F,0x530,0x531,0x532,0x533,0x534,0x535,0x536,0x537,0x538,0x539,
                          0x53A,0x53B,0x53C,0x53D,0x53E,0x53F,0x540,0x541,0x542,0x543,0x544,0x545,0x546,0x547,0x548,0x549,
                          0x54A,0x54B,0x54C,0x54D,0x54E,0x54F};

    for(int i=0; i < 118; i++) {
        this->Cl_camera->send_TC(this->base_ad | address_base_imx[i], reg_values.at(i));

     }


}


QByteArray IMX253::get_test_regs() {

    QByteArray IMX_regs_ar;

    //qDebug() << "send_TM 128";
//    IMX_regs_ar = this->Cl_camera->send_TM(128, this->base_ad | IMX_CONF_REG_00);
     int address[17] = {0x438,0x439,0x43A,0x43C,0x43D,0x43E,0x43F,0x440,0x441,0x444,0x445,0x446,0x447,0x448,0x449,0x454,0x455};

    for(int i=0; i < 17; i++) {
      IMX_regs_ar[i]= this->Cl_camera->send_TM(this->base_ad | address[i]);

    }

    return IMX_regs_ar;
}

void IMX253::set_test_regs(QByteArray reg_values) {


    int address[17]  = {0x438,0x439,0x43A,0x43C,0x43D,0x43E,0x43F,0x440,0x441,0x444,0x445,0x446,0x447,0x448,0x449,0x454,0x455};

    for(int i=0; i < 17; i++) {
        this->Cl_camera->send_TC(this->base_ad | address[i], reg_values.at(i));

     }


}


void IMX253::configFREQ(quint8 DDR_mode_disable){

      this->Cl_camera->send_TC(this->base_ad | IMX_CONF_FREQ,  DDR_mode_disable);

}

double IMX253::Get_IMX_Bandwidth(quint8 nb_chan, quint8 nb_bits){

    double Imx_Mps;

    if (nb_chan == 16)
        Imx_Mps = 9504;
    else if (nb_chan == 8)
        Imx_Mps = 4752;
    else // 4
        Imx_Mps = 2376;

    return Imx_Mps/nb_bits;


//      if (DDR_mode_disable == 0) { //FREQ = 0
//        if (imx_in_freq == 74.25) {
//            if (nb_chan == 16)
//                return 1188;
//            else if (nb_chan == 8)
//                return 594;
//            else
//                return 297;
//        }else if (imx_in_freq == 54.){
//            if (nb_chan == 16)
//                 return 864;
//            else if (nb_chan == 8)
//                 return 432;
//            else
//                 return 216;
//        }else{ // imx_in_freq 37.125
//            if (nb_chan == 16)
//                return 594;
//            else if (nb_chan == 8)
//                return 297;
//            else
//                return 148.5;
//        }
//    }else {
//        if (imx_in_freq == 74.25) {
//            if (nb_chan == 16)
//                return 594;
//            else if (nb_chan == 8)
//                return 297;
//            else
//                return 148.5;
//        }else if (imx_in_freq == 54.){
//            if (nb_chan == 16)
//                return 432;
//            else if (nb_chan == 8)
//                return 216;
//            else
//                return 108;
//        }else{ // imx_in_freq 37.125
//            if (nb_chan == 16)
//                return 297;
//            else if (nb_chan == 8)
//                return 148.5;
//            else
//                return 74.25;
//        }
//    }


}

double IMX253::Get_FPS( quint32 height, double imx_in_freq, quint8 nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable){

    double FPS;
//    if (DDR_mode_disable == 0) { //FREQ = 0
//        if (nb_bits_sensor == 8) {
//            if (nb_chan == 16)
//                FPS = 68.3;
//            else if (nb_chan == 8)
//                FPS = 42.6;
//            else
//                FPS = 22.2;
//        }else if (nb_bits_sensor == 10){
//            if (nb_chan == 16)
//                FPS = 64.6;
//            else if (nb_chan == 8)
//                FPS = 34.6;
//            else
//                FPS = 17.9;
//        }else{ // nb_bit = 12
//            if (nb_chan == 16)
//                FPS = 46.4;
//            else if (nb_chan == 8)
//                FPS = 29.1;
//            else
//                FPS = 15.0;
//        }
//    }else {
//        if (nb_bits_sensor == 8) {
//            if (nb_chan == 16)
//                FPS = 41.6;
//            else if (nb_chan == 8)
//                FPS = 21.9;
//            else
//                FPS = 11.2;
//        }else if (nb_bits_sensor == 10){
//            if (nb_chan == 16)
//                FPS = 33.9;
//            else if (nb_chan == 8)
//                FPS = 17.7;
//            else
//                FPS = 9;
//        }else{ // nb_bit = 12
//            if (nb_chan == 16)
//                FPS = 28.7;
//            else if (nb_chan == 8)
//                FPS = 14.9;
//            else
//                FPS = 7.6;
//        }
//    }
    double H_period = Get_H_period_IMX(imx_in_freq, nb_bits_sensor,nb_chan, DDR_mode_disable);
//    qDebug () << "H_period   = " + QString::number(H_period);

    FPS = 1/((height+54)*(H_period/1000000));
//    qDebug () << "FPS  = " + QString::number(FPS);
    return FPS;
}



quint16 IMX253::GetHMAX(quint8   nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable){

    quint16 HMAX;

     if (DDR_mode_disable == 0) {
        if (nb_bits_sensor == 8) {
            if (nb_chan == 16)
                HMAX = 0x163;
            else if (nb_chan == 8)
                HMAX = 0x238;
            else
                HMAX = 0x444;
        }else if (nb_bits_sensor == 10){
            if (nb_chan == 16)
                HMAX = 0x177;
            else if (nb_chan == 8)
                HMAX = 0x2BC;
            else
                HMAX = 0x54B;
        }else{ // nb_bit = 12
            if (nb_chan == 16)
                HMAX = 0x201;
            else if (nb_chan == 8)
                HMAX = 0x33F;
            else
                HMAX = 0x64E;
        }
    }else{
         if (nb_bits_sensor == 8) {
             if (nb_chan == 16)
                 HMAX = 0x246;
             else if (nb_chan == 8)
                 HMAX = 0x450;
             else
                 HMAX = 0x868;
         }else if (nb_bits_sensor == 10){
             if (nb_chan == 16)
                 HMAX = 0x2CB;
             else if (nb_chan == 8)
                 HMAX = 0x555;
             else
                 HMAX = 0xA6E;
         }else{ // nb_bit = 12
             if (nb_chan == 16)
                 HMAX = 0x34B;
             else if (nb_chan == 8)
                 HMAX = 0x64A;
             else
                 HMAX = 0xC78;
         }
     }

return HMAX;
}

double IMX253::Get_exposure_MAX(quint8 imx_in_freq, quint16 height, quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable){

    double H_period = Get_H_period_IMX(imx_in_freq, nb_bits_sensor, nb_chan,  DDR_mode_disable);

    // double H_period = this->imx253->Get_H_period_IMX(quint8(this->IMX_freq_div->currentIndex()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());

    double exposure_MAX = (H_period * (height+VB1+frameInfo -20)+ 14.26)/1000;

    // qDebug().noquote() << "H_period     = " + QString::number(H_period) + " us";
    // qDebug().noquote() << "exposure MAX = " + QString::number(exposure_MAX) + " ms";

    return exposure_MAX;
}




double IMX253::Get_H_period_IMX(quint8 imx_in_freq,  quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable)
{
    double INCK_PerPeriod;

    /// codage tableau datasheet.
   if (DDR_mode_disable == 1) {
    if (imx_in_freq == 0){ // freq = 74.25
        if (nb_bits_sensor == 8 ){
            if (nb_chan == 4)
                INCK_PerPeriod = 2152;
            else if(nb_chan == 8)
               INCK_PerPeriod = 1104;
            else
               INCK_PerPeriod = 582;
        }
        else if (nb_bits_sensor == 10){
            if (nb_chan == 4)
               INCK_PerPeriod = 2670;
            else if(nb_chan == 8)
               INCK_PerPeriod = 1365;
            else
               INCK_PerPeriod = 715;
        }
        else {
            if (nb_chan == 4)
               INCK_PerPeriod = 3192;
            else if(nb_chan == 8)
               INCK_PerPeriod = 1626;
            else
               INCK_PerPeriod = 843;
        }
    }
    else if  (imx_in_freq == 1){ // FREQ 54
        if (nb_bits_sensor == 8 ){
            if (nb_chan == 4)
               INCK_PerPeriod = 1565.1;
            else if(nb_chan == 8)
               INCK_PerPeriod = 802.9;
            else
               INCK_PerPeriod = 423.3;
        }
        else if (nb_bits_sensor == 10){
            if (nb_chan == 4)
               INCK_PerPeriod = 1941.8;
            else if(nb_chan == 8)
               INCK_PerPeriod = 992.7;
            else
               INCK_PerPeriod = 520.0;
        }
        else {
            if (nb_chan == 4)
               INCK_PerPeriod = 2321.5;
            else if(nb_chan == 8)
               INCK_PerPeriod = 1182.5;
            else
               INCK_PerPeriod = 613.1;
        }
    }
   else { // 37.125 MHz

        if (nb_bits_sensor == 8 ){
            if (nb_chan == 4)
               INCK_PerPeriod = 1076.0;
            else if(nb_chan == 8)
               INCK_PerPeriod = 552;
            else
               INCK_PerPeriod = 291;
        }
        else if (nb_bits_sensor == 10){
            if (nb_chan == 4)
               INCK_PerPeriod = 1335;
            else if(nb_chan == 8)
               INCK_PerPeriod = 682.5;
            else
               INCK_PerPeriod = 357.5;
        }
        else {
            if (nb_chan == 4)
               INCK_PerPeriod = 1596;
            else if(nb_chan == 8)
               INCK_PerPeriod = 813;
            else
               INCK_PerPeriod = 421.5;
        }
      }
    }
   else { // DDR_mode_disable == 1
       if (imx_in_freq == 0){ // 74.125
           if (nb_bits_sensor == 8 ){
               if (nb_chan == 4)
                  INCK_PerPeriod = 1092;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 568;
               else
                  INCK_PerPeriod = 355;
           }
           else if (nb_bits_sensor == 10){
               if (nb_chan == 4)
                  INCK_PerPeriod = 1355;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 700;
               else
                  INCK_PerPeriod = 375;
           }
           else {
               if (nb_chan == 4)
                  INCK_PerPeriod = 1614;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 831;
               else
                  INCK_PerPeriod = 522;
           }
       }
       else if  (imx_in_freq == 1){ // 54
           if (nb_bits_sensor == 8 ){
               if (nb_chan == 4)
                  INCK_PerPeriod = 794.2;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 413.1;
               else
                  INCK_PerPeriod = 258.2;
           }
           else if (nb_bits_sensor == 10){
               if (nb_chan == 4)
                  INCK_PerPeriod = 985.5;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 509.1;
               else
                  INCK_PerPeriod = 272.7;
           }
           else {
               if (nb_chan == 4)
                  INCK_PerPeriod = 1173.8;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 604.4;
               else
                  INCK_PerPeriod = 379.6;
           }
       }
      else { // 37.125 MHz

           if (nb_bits_sensor == 8 ){
               if (nb_chan == 4)
                  INCK_PerPeriod = 546;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 284;
               else
                  INCK_PerPeriod = 117.5;
           }
           else if (nb_bits_sensor == 10){
               if (nb_chan == 4)
                  INCK_PerPeriod = 677.5;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 350.0;
               else
                  INCK_PerPeriod = 187.5;
           }
           else {
               if (nb_chan == 4)
                  INCK_PerPeriod = 807;
               else if(nb_chan == 8)
                  INCK_PerPeriod = 415.5;
               else
                  INCK_PerPeriod = 261.0;
           }
        }
     }

   double imx_in_freq_MHz;
   if (imx_in_freq == 0)
       imx_in_freq_MHz = 74.25;
   else if (imx_in_freq == 0)
       imx_in_freq_MHz = 54;
   else
       imx_in_freq_MHz = 37.125;

//   qDebug () << "INCK_PerPeriod   = " + QString::number(INCK_PerPeriod);
//   qDebug () << "imx_in_freq_MHz  = " + QString::number(imx_in_freq_MHz);

   return INCK_PerPeriod / (imx_in_freq_MHz );


}



// Convert exposure time in us to quint32 register value as defined in CMV datasheet
quint32 IMX253::exposure_f_to_int(double exposure_time, double IMX_in_freq, quint8 fot_length) {
    double clk_per = 1 / IMX_in_freq;

    //qDebug () << "IMX_in_freq = " + QString::number(IMX_in_freq) + "MHz";
    //qDebug () << "clk_per = " + QString::number(clk_per) + "us";

    qint32 exp_time_int = static_cast<qint32>((exposure_time / (129.0*clk_per) - 0.43 * fot_length)); // a modif

    if (exp_time_int < 0) {
        return 0;
    }

    return static_cast<quint32>(exp_time_int);
}
