#include "imx.h"

IMX::IMX(CameraLink *Cl_camera, quint32 base_ad, quint8 cam_fw_type)
{
    this->Cl_camera = Cl_camera;
    this->base_ad = base_ad;
    if (cam_fw_type == CAM_FW_CASPEX_IR)
        this->cam_type = 1;
    else
        this->cam_type = 0;



}

quint8 IMX::Get_SensorType()
{

    this->enable_standby_mode(false);

    // Sleep(10);

    qDebug() << "Check IMX VERSION...";
    QByteArray IMX_version = this->Cl_camera->send_TM(2,this->base_ad | IMX_VERSION_LSB[this->cam_type]);

    quint8 sensor_type = 0;
     this->enable_standby_mode(true);
    if (cam_type == 0){
     if (IMX_version.at(0) == 0x40) {
         if (((IMX_version.at(1) & 0xff) == 0xbf) // LLR (monochrome)
          || ((IMX_version.at(1) & 0xff) == 0x3f)) { // LQR (color)

            if ((IMX_version.at(1) & 0xff) == 0x3f) {
              qDebug().noquote() << "IMX LQR (color version)";
            }
            else {
              qDebug().noquote() << "IMX LLR (monochrome version)";
            }
            sensor_type= 3;
        }
     }
     else {
         qDebug() << "No sensor found!!!!!! : IMX_version: 0x" + QString::number(IMX_version.at(0),16) + " and 0x" + QString::number(IMX_version.at(1),16);
         sensor_type= 0;
     }
    }else { // caspex IR

        if ((IMX_version.at(0)& 0xff) == 0xC0) {
            if ((IMX_version.at(1) & 0xff) == 0xfb){ //  IMX990
               qDebug().noquote() << "IMX990 detected";
                sensor_type= 3;
        }
        }
        else {
            qDebug() << "No sensor found!!!!!! : IMX_version: 0x" + QString::number(IMX_version.at(0),16) + " and 0x" + QString::number(IMX_version.at(1),16);
            sensor_type= 0;
        }
    }

 return sensor_type;

}

void IMX::enable_standby_mode(bool enable)
{
//    if (enable == true) {
//        qDebug().noquote() << "ENABLE Standby mode";
//    }
//    else {
//        qDebug().noquote() << "DISABLE Standby mode";
//    }
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STANDBY[this->cam_type],  quint8(enable));
}

void IMX::enable_PGMODE(bool enable)
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

void IMX::disable_master_mode()
{
    this->Cl_camera->send_TC(this->base_ad | IMX_XMSTA[this->cam_type], 0x1 );
}


void IMX::config(quint8   nb_chan, // Number of data channels
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
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_HMAX_LSB[this->cam_type],  quint8 (HMAX));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_HMAX_MSB[this->cam_type],  quint8 (HMAX>>8));

//    qDebug().noquote() << "HMAX = 0x" + QString::number(HMAX,16);

    //NB channel
    if (cam_type == 0){
        if (nb_chan==4){
           this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL[this->cam_type], 0x30);
           this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS[this->cam_type],  0x20);
        }
       else if(nb_chan==8){
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL[this->cam_type], 0x10);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS[this->cam_type],  0x10);
        }
        else {
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL[this->cam_type], 0x90);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS[this->cam_type],  0x00);
        }
    }
    else {
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_OPORTSEL[this->cam_type], 0x03);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_STBLVDS[this->cam_type],  0x02);
    }

    //TOUTxSEL
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TOUTxSEL[this->cam_type],  0x0);
    //TRIG TOUTxSEL
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TRIG_TOUTxSEL[this->cam_type],  0x0);

    //PULSE
    quint8 i;
    quint8 y;

    // this->spw_camera->send_TC(this->base_ad | IMX_CONF_PULSE1_CONF,  0x1);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE1_CONF[this->cam_type],  0x0);

    for (i= 1; i<3;i++) {
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP[this->cam_type])+i,  0x0);
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN[this->cam_type])+i,  0x0);
    }
    // this->spw_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP)+0,  0x03);
    // this->spw_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN)+0,  0x60);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_UP[this->cam_type])+0,  0x0);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE1_DN[this->cam_type])+0,  0x0);

    // this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF,  0x05);
    //this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF,  0x09);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_PULSE2_CONF[this->cam_type],  0x00);
    for (y= 1; y<3;y++) {
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP[this->cam_type])+y,  0x0);
       this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN[this->cam_type])+y,  0x0);
    }

    // this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP)+0,  0x03);
    // this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN)+0,  0x05);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_UP[this->cam_type])+0,  0x0);
    this->Cl_camera->send_TC((this->base_ad | IMX_CONF_PULSE2_DN[this->cam_type])+0,  0x0);

    Sleep(10);


    if (cam_type == 0){
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
    }
    else {

//        this->Cl_camera->send_TC(this->base_ad | 0x206, 0x80);
//        this->Cl_camera->send_TC(this->base_ad | 0x207, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x240, 0x02);
//        this->Cl_camera->send_TC(this->base_ad | 0x254, 0x11);
//        this->Cl_camera->send_TC(this->base_ad | 0x256, 0x1B);
//        this->Cl_camera->send_TC(this->base_ad | 0x258, 0x21);
//        this->Cl_camera->send_TC(this->base_ad | 0x2F0, 0x01);

//        this->Cl_camera->send_TC(this->base_ad | 0x400, 0x45);
//        this->Cl_camera->send_TC(this->base_ad | 0x444, 0x06);
//        this->Cl_camera->send_TC(this->base_ad | 0x446, 0x06);
//        this->Cl_camera->send_TC(this->base_ad | 0x44C, 0x06);
//        this->Cl_camera->send_TC(this->base_ad | 0x4D0, 0x06);
//        this->Cl_camera->send_TC(this->base_ad | 0x4E0, 0x06);

//        this->Cl_camera->send_TC(this->base_ad | 0x502, 0x03);
//        this->Cl_camera->send_TC(this->base_ad | 0x580, 0x07);
//        this->Cl_camera->send_TC(this->base_ad | 0x590, 0x07);

//        this->Cl_camera->send_TC(this->base_ad | 0x630, 0x02);

//        this->Cl_camera->send_TC(this->base_ad | 0x728, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x72A, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x72C, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x72E, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x740, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x744, 0x33);
//        this->Cl_camera->send_TC(this->base_ad | 0x7A0, 0xA0);
//        this->Cl_camera->send_TC(this->base_ad | 0x788, 0x32);
//        this->Cl_camera->send_TC(this->base_ad | 0x798, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x7C0, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x7EE, 0x08);

//        this->Cl_camera->send_TC(this->base_ad | 0x804, 0x23);
//        this->Cl_camera->send_TC(this->base_ad | 0x806, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x808, 0x3D);
//        this->Cl_camera->send_TC(this->base_ad | 0x80A, 0x0C);
//        this->Cl_camera->send_TC(this->base_ad | 0x80C, 0x52);
//        this->Cl_camera->send_TC(this->base_ad | 0x80E, 0x02);
//        this->Cl_camera->send_TC(this->base_ad | 0x816, 0x05);
//        this->Cl_camera->send_TC(this->base_ad | 0x826, 0x46);
//        this->Cl_camera->send_TC(this->base_ad | 0x83C, 0x23);
//        this->Cl_camera->send_TC(this->base_ad | 0x83E, 0x2C);
//        this->Cl_camera->send_TC(this->base_ad | 0x840, 0x3D);
//        this->Cl_camera->send_TC(this->base_ad | 0x844, 0x52);
//        this->Cl_camera->send_TC(this->base_ad | 0x846, 0x0C);
//        this->Cl_camera->send_TC(this->base_ad | 0x84E, 0x3D);
//        this->Cl_camera->send_TC(this->base_ad | 0x852, 0x1F);
//        this->Cl_camera->send_TC(this->base_ad | 0x85E, 0x3E);
//        this->Cl_camera->send_TC(this->base_ad | 0x874, 0x23);
//        this->Cl_camera->send_TC(this->base_ad | 0x876, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x878, 0x3D);
//        this->Cl_camera->send_TC(this->base_ad | 0x87A, 0x0C);
//        this->Cl_camera->send_TC(this->base_ad | 0x87C, 0x52);
//        this->Cl_camera->send_TC(this->base_ad | 0x87E, 0x02);
//        this->Cl_camera->send_TC(this->base_ad | 0x886, 0x05);
//        this->Cl_camera->send_TC(this->base_ad | 0x896, 0x16);

//        this->Cl_camera->send_TC(this->base_ad | 0x962, 0x2D);

//        this->Cl_camera->send_TC(this->base_ad | 0x1060, 0x60);
//        this->Cl_camera->send_TC(this->base_ad | 0x1061, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1062, 0x8C);
//        this->Cl_camera->send_TC(this->base_ad | 0x1063, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1070, 0x60);
//        this->Cl_camera->send_TC(this->base_ad | 0x1071, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1072, 0x8C);
//        this->Cl_camera->send_TC(this->base_ad | 0x1073, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1092, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1093, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x10F0, 0x61);
//        this->Cl_camera->send_TC(this->base_ad | 0x10F1, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x10F2, 0x8B);
//        this->Cl_camera->send_TC(this->base_ad | 0x10F3, 0x00);


//        this->Cl_camera->send_TC(this->base_ad | 0x1104, 0x61);
//        this->Cl_camera->send_TC(this->base_ad | 0x1105, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1106, 0x8B);
//        this->Cl_camera->send_TC(this->base_ad | 0x1107, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1124, 0xB2);
//        this->Cl_camera->send_TC(this->base_ad | 0x1126, 0xD0);
//        this->Cl_camera->send_TC(this->base_ad | 0x1164, 0xB2);
//        this->Cl_camera->send_TC(this->base_ad | 0x1166, 0xD0);
//        this->Cl_camera->send_TC(this->base_ad | 0x1174, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x1176, 0x53);
//        this->Cl_camera->send_TC(this->base_ad | 0x11B6, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x11B7, 0x00);

//        this->Cl_camera->send_TC(this->base_ad | 0x1218, 0x02);
//        this->Cl_camera->send_TC(this->base_ad | 0x121A, 0x52);
//        this->Cl_camera->send_TC(this->base_ad | 0x1248, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1249, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x124E, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x124F, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1258, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1259, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x125E, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x125F, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x126A, 0x70);
//        this->Cl_camera->send_TC(this->base_ad | 0x126B, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x126C, 0xBA);
//        this->Cl_camera->send_TC(this->base_ad | 0x124D, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1298, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1299, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x129A, 0x02);
//        this->Cl_camera->send_TC(this->base_ad | 0x129B, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x129C, 0x03);
//        this->Cl_camera->send_TC(this->base_ad | 0x129D, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x12C8, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x12C9, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x12CA, 0x15);
//        this->Cl_camera->send_TC(this->base_ad | 0x12CB, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x12CC, 0x16);
//        this->Cl_camera->send_TC(this->base_ad | 0x12CD, 0x00);


//        this->Cl_camera->send_TC(this->base_ad | 0x13BC, 0x15);
//        this->Cl_camera->send_TC(this->base_ad | 0x13BC, 0x09);
//        this->Cl_camera->send_TC(this->base_ad | 0x13C0, 0x41);
//        this->Cl_camera->send_TC(this->base_ad | 0x13C1, 0x41);
//        this->Cl_camera->send_TC(this->base_ad | 0x13C2, 0x41);

//        this->Cl_camera->send_TC(this->base_ad | 0x1407, 0x1F);
//        this->Cl_camera->send_TC(this->base_ad | 0x146F, 0x22);
//        this->Cl_camera->send_TC(this->base_ad | 0x1488, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x148C, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x1490, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x1494, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x1498, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x149C, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14A0, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x14A4, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14A8, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x14AC, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14B0, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x14B4, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14B8, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x14BC, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14C0, 0x97);
//        this->Cl_camera->send_TC(this->base_ad | 0x14C4, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x14D4, 0x34);


//        this->Cl_camera->send_TC(this->base_ad | 0x1500, 0x3B);
//        this->Cl_camera->send_TC(this->base_ad | 0x1522, 0x3F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1542, 0x3F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1560, 0x65);
//        this->Cl_camera->send_TC(this->base_ad | 0x1562, 0x74);
//        this->Cl_camera->send_TC(this->base_ad | 0x1564, 0xF6);
//        this->Cl_camera->send_TC(this->base_ad | 0x1566, 0x05);
//        this->Cl_camera->send_TC(this->base_ad | 0x1570, 0x54);
//        this->Cl_camera->send_TC(this->base_ad | 0x1580, 0x40);
//        this->Cl_camera->send_TC(this->base_ad | 0x1588, 0x41);
//        this->Cl_camera->send_TC(this->base_ad | 0x15A1, 0x15);
//        this->Cl_camera->send_TC(this->base_ad | 0x15A8, 0x34);
//        this->Cl_camera->send_TC(this->base_ad | 0x15AB, 0x40);
//        this->Cl_camera->send_TC(this->base_ad | 0x15B0, 0x38);
//        this->Cl_camera->send_TC(this->base_ad | 0x15BE, 0x3D);
//        this->Cl_camera->send_TC(this->base_ad | 0x15C0, 0x34);
//        this->Cl_camera->send_TC(this->base_ad | 0x15C8, 0x38);
//        this->Cl_camera->send_TC(this->base_ad | 0x15DB, 0x1F);


//        this->Cl_camera->send_TC(this->base_ad | 0x1618, 0x76);
//        this->Cl_camera->send_TC(this->base_ad | 0x161A, 0x62);
//        this->Cl_camera->send_TC(this->base_ad | 0x1628, 0x77);
//        this->Cl_camera->send_TC(this->base_ad | 0x162A, 0x63);
//        this->Cl_camera->send_TC(this->base_ad | 0x1638, 0x78);
//        this->Cl_camera->send_TC(this->base_ad | 0x163A, 0x64);
//        this->Cl_camera->send_TC(this->base_ad | 0x1644, 0x65);
//        this->Cl_camera->send_TC(this->base_ad | 0x1646, 0x5D);
//        this->Cl_camera->send_TC(this->base_ad | 0x164C, 0x66);
//        this->Cl_camera->send_TC(this->base_ad | 0x164E, 0x5E);
//        this->Cl_camera->send_TC(this->base_ad | 0x1654, 0x67);
//        this->Cl_camera->send_TC(this->base_ad | 0x1656, 0x5F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1668, 0x5A);
//        this->Cl_camera->send_TC(this->base_ad | 0x166A, 0x50);
//        this->Cl_camera->send_TC(this->base_ad | 0x1678, 0x4E);
//        this->Cl_camera->send_TC(this->base_ad | 0x167A, 0x46);
//        this->Cl_camera->send_TC(this->base_ad | 0x1688, 0x65);
//        this->Cl_camera->send_TC(this->base_ad | 0x1692, 0x52);
//        this->Cl_camera->send_TC(this->base_ad | 0x16B0, 0x31);
//        this->Cl_camera->send_TC(this->base_ad | 0x16B2, 0x39);
//        this->Cl_camera->send_TC(this->base_ad | 0x16C0, 0x31);
//        this->Cl_camera->send_TC(this->base_ad | 0x16C2, 0x39);
//        this->Cl_camera->send_TC(this->base_ad | 0x16CD, 0x20);
//        this->Cl_camera->send_TC(this->base_ad | 0x16EC, 0x44);


//        this->Cl_camera->send_TC(this->base_ad | 0x1764, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1766, 0x2C);
//        this->Cl_camera->send_TC(this->base_ad | 0x176C, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x176E, 0x2D);
//        this->Cl_camera->send_TC(this->base_ad | 0x1773, 0xFF);

//        this->Cl_camera->send_TC(this->base_ad | 0x1828, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1829, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x182A, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1830, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1831, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1832, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1838, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1839, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x183A, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1840, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1841, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1842, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1848, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1849, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x184A, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1850, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1851, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1852, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1858, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1859, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x185A, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1860, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x1861, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x1862, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18A8, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18A9, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18AA, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18B0, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18B1, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18B2, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18B8, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18B9, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18BA, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18C0, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18C1, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18C2, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18C8, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18C9, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18CA, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18D0, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18D1, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18D2, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18D8, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18D9, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18DA, 0x00);

//        this->Cl_camera->send_TC(this->base_ad | 0x18E0, 0xFF);
//        this->Cl_camera->send_TC(this->base_ad | 0x18E1, 0x0F);
//        this->Cl_camera->send_TC(this->base_ad | 0x18E2, 0x00);

//        this->Cl_camera->send_TC(this->base_ad | 0x18E9, 0x78);
//        this->Cl_camera->send_TC(this->base_ad | 0x18EC, 0x9A);
//        this->Cl_camera->send_TC(this->base_ad | 0x18EE, 0x00);

//        this->Cl_camera->send_TC(this->base_ad | 0x18F2, 0x28);
//        this->Cl_camera->send_TC(this->base_ad | 0x18F3, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x18F4, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x18F6, 0x98);
//        this->Cl_camera->send_TC(this->base_ad | 0x18F8, 0x2A);
//        this->Cl_camera->send_TC(this->base_ad | 0x18F9, 0x01);


//        this->Cl_camera->send_TC(this->base_ad | 0x1900, 0x65);
//        this->Cl_camera->send_TC(this->base_ad | 0x190A, 0xF6);
//        this->Cl_camera->send_TC(this->base_ad | 0x1918, 0x65);
//        this->Cl_camera->send_TC(this->base_ad | 0x191A, 0x6A);

//        this->Cl_camera->send_TC(this->base_ad | 0x1920, 0xF6);
//        this->Cl_camera->send_TC(this->base_ad | 0x1922, 0xFB);
//        this->Cl_camera->send_TC(this->base_ad | 0x1923, 0x00);

//        this->Cl_camera->send_TC(this->base_ad | 0x192E, 0x73);
//        this->Cl_camera->send_TC(this->base_ad | 0x1930, 0x18);
//        this->Cl_camera->send_TC(this->base_ad | 0x1932, 0x42);
//        this->Cl_camera->send_TC(this->base_ad | 0x1938, 0x42);
//        this->Cl_camera->send_TC(this->base_ad | 0x193A, 0x18);

//        this->Cl_camera->send_TC(this->base_ad | 0x1950, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1951, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x1952, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x1953, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x1954, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x1955, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1956, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1957, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1958, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1959, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x195A, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x195B, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x195C, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x195D, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x195E, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x195F, 0x08);
//        this->Cl_camera->send_TC(this->base_ad | 0x1960, 0x1A);
//        this->Cl_camera->send_TC(this->base_ad | 0x1961, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x1962, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x1963, 0x1C);
//        this->Cl_camera->send_TC(this->base_ad | 0x1978, 0x3E);
//        this->Cl_camera->send_TC(this->base_ad | 0x197A, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x197B, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x1982, 0x2B);
//        this->Cl_camera->send_TC(this->base_ad | 0x1988, 0xb6);
//        this->Cl_camera->send_TC(this->base_ad | 0x1989, 0x00);
//        this->Cl_camera->send_TC(this->base_ad | 0x198A, 0x2B);
//        this->Cl_camera->send_TC(this->base_ad | 0x198B, 0x01);
//        this->Cl_camera->send_TC(this->base_ad | 0x1998, 0x0A);
//        this->Cl_camera->send_TC(this->base_ad | 0x19B8, 0x45);
//        this->Cl_camera->send_TC(this->base_ad | 0x19C2, 0x46);

    }

    // Configure resolution of the sensor
    setXTrigEnable(xtrig_en); // false SHS, true XTRIG
    mode_config(nb_bits);    // 8 or 10 or 12
    configFREQ(DDR_mode_disable);


    // Sleep(1000);

}

void IMX::pll_config(quint8 freq_in)
{
    if (cam_type == 0){ // IMX253
        if (freq_in == 0){ // 74.25
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  16);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  0);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  16);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  0);
        }else if (freq_in == 1){ //54
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  22);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  0);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  22);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  0);
        }else { //37.125
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  16);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  2);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  16);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  2);
        }
    }
    else {
        if (freq_in == 0){ // 74.25
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  0x52);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  0x20);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  0x52);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  0x20);
        }else if (freq_in == 1){ //54
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  0x50);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  0x16);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  0x50);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  0x16);
        }else { //37.125
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL0[this->cam_type],  0x50);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL1[this->cam_type],  0x20);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL2[this->cam_type],  0x50);
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_INCKSEL3[this->cam_type],  0x20);
        }
    }
}

///*******************************************************/
void IMX::mode_config(quint8 bit_mode)
{
    //ADBBIT
    if (bit_mode == 8){
        if (cam_type == 0)
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT[this->cam_type],  2);
        else
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT[this->cam_type],  2<<5);

        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT[this->cam_type],  2);

    }else if(bit_mode == 12){
        if (cam_type == 0)
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT[this->cam_type],  1);
        else
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT[this->cam_type],  1<<5);

         this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT[this->cam_type],  1);
    }else {
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ADBIT[this->cam_type],  0);
        this->Cl_camera->send_TC(this->base_ad | IMX_CONF_ODBIT[this->cam_type],  0);
    }
}

void IMX::setXTrigEnable(bool mode){

    if (mode==true){
//    qDebug().noquote() << "ATTENTION FAST TRIGGER MODE !!!!";

    this->Cl_camera->send_TC(this->base_ad | IMX_TRIGEN[this->cam_type], 0x1);
    this->Cl_camera->send_TC(this->base_ad | IMX_LOWLAGTRG[this->cam_type], 0x1);
//    qDebug().noquote() << "###############################################";
    }
    else{

        this->Cl_camera->send_TC(this->base_ad | IMX_TRIGEN[this->cam_type], 0x0);
        this->Cl_camera->send_TC(this->base_ad | IMX_LOWLAGTRG[this->cam_type], 0x0);
    }
}

double IMX::Get_temperature() {
    enable_standby_mode(false);
    // TMD_LATCH (Update value of TMOUT)
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TMOUT[this->cam_type] ,  0x1);

    QByteArray data = this->Cl_camera->send_TM(2, this->base_ad | SENS_IMX_TEMP[this->cam_type]);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;

    // TMD_LATCH (Set to 0 before next reading)
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_TMOUT[this->cam_type],  0x0);

    quint16 temp_val = (data0 | data1);

    double temp_celsius = 246.312 - 0.304 * temp_val;
//    qDebug().noquote() << "Temp °C = "+ QString::number(temp_celsius);
    enable_standby_mode(true);
    return temp_celsius;
}

void  IMX::Set_Offset(quint16 offset) {
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_BLKLVL[this->cam_type]  ,  quint8(offset));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_BLKLVL[this->cam_type]+1,  quint8(offset>>8));
}

quint16 IMX::Get_Offset() {
    QByteArray data = this->Cl_camera->send_TM(2, this->base_ad | IMX_CONF_BLKLVL[this->cam_type]);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;
    return (data0 | data1);
}

bool IMX::Get_Test_Mode(){

    QByteArray data =  this->Cl_camera->send_TM(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type]);

    if (data.at(0) % 2 == 1) // bit LSB à 1 => test mode enable
        return true;
    else
        return false;
}

void IMX::EnableTestMode(bool enable){

//    QByteArray data =  this->Cl_camera->send_TM(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type]);
//    if (cam_type == 0) {
//    if (enable == true)
//        this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type], ( quint8(data.at(0)) | 0x01) );
//    else
//        this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type], ( quint8(data.at(0)) & 0xFE) );
//    }
//    else
//    {
        if (enable == true){
            this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type], ( 0x07));
            this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type]+1, (0x03));
        }
        else
            this->Cl_camera->send_TC(1, this->base_ad | IMX_CONF_TEST_EN[this->cam_type], (0x02));
//    }





}

quint16 IMX::Get_ADC() {
    QByteArray data = this->Cl_camera->send_TM(2,this->base_ad | IMX_CONF_GAIN_LSB[this->cam_type]);
    quint16 data1 = static_cast<quint16>((data.at(1) << 8)) & 0xFF00;
    quint16 data0 = static_cast<quint16>((data.at(0) << 0)) & 0x00FF;
return (data0 | data1);
}

//void  IMX::Set_Vramp(quint32 value) {
//}
void  IMX::Set_SHS(quint32 value) {

//    quint8 SHS[3];
//    SHS[0] =(value >>  0) & 0xff;
//    SHS[1] =(value >>  8) & 0xff;
//    SHS[2] =(value >> 16) & 0xff;
//    qDebug().noquote() << "Set SHS to 0x" + QString::number(value,16) + ", ("+QString::number(value,10) + ")";
    this->Cl_camera->send_TC(3,this->base_ad |  IMX_CONF_SHS_EXP_TIME[this->cam_type], value);
}

void  IMX::Set_ADC(quint32 value) {

//    qDebug().noquote() << "Set GAIN to 0x" + QString::number(value,16) + "starting at address 0x" + QString::number(this->base_ad | IMX_CONF_GAIN_LSB,16);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_GAIN_LSB[this->cam_type], quint8 (value));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_GAIN_MSB[this->cam_type], quint8 (value>>8));
}

void IMX::Set_VMAX(quint16 height){

    quint32 value = height + VB1[cam_type] + frameInfo[cam_type];
    qDebug().noquote() << "Set VMAX to " +QString::number(value,10);
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_LSB[this->cam_type],  quint8 (value));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MID[this->cam_type],  quint8 (value>>8));
    this->Cl_camera->send_TC(this->base_ad | IMX_CONF_VMAX_MSB[this->cam_type],  quint8 (value>>16));

    //set ROI enable
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_ON_AD0[this->cam_type],  3);
    //Set ROI size height
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WV_LSB[this->cam_type],  quint8 (height));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WV_MSB[this->cam_type],  quint8 (height>>8));
    //Set ROI size width
    quint16 width;
    if (cam_type == 0) // CINEMA
        width = 4112;
    else
        width = 1392;

    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WH_LSB[this->cam_type],  quint8 (width));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_WH_MSB[this->cam_type],  quint8 (width>>8));

}

void IMX::Set_Offset_y(quint16 offset_y)
{

    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_PV_LSB[this->cam_type],  quint8 (offset_y));
    this->Cl_camera->send_TC(this->base_ad | IMX_ROI_1_PV_MSB[this->cam_type],  quint8 (offset_y>>8));

}

//bool IMX::setTrainingPattern(quint16 training_pattern) {
//    QByteArray data;
//    data.clear();
//    data.append(static_cast<qint8>(training_pattern & 0xff));
//    data.append(static_cast<qint8>(training_pattern >> 8));

//    //Set training pattern
//    //return this->Cl_camera->send_TC(this->base_ad | IMX_CONF_REG_78, data);
//    return 0;
//}

QByteArray IMX::get_IMX_regs() {

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

void IMX::set_IMX_regs(QByteArray reg_values) {

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


QByteArray IMX::get_test_regs() {

    QByteArray IMX_regs_ar;

    //qDebug() << "send_TM 128";
//    IMX_regs_ar = this->Cl_camera->send_TM(128, this->base_ad | IMX_CONF_REG_00);
     int address[17] = {0x438,0x439,0x43A,0x43C,0x43D,0x43E,0x43F,0x440,0x441,0x444,0x445,0x446,0x447,0x448,0x449,0x454,0x455};

    for(int i=0; i < 17; i++) {
      IMX_regs_ar[i]= this->Cl_camera->send_TM(this->base_ad | address[i]);

    }

    return IMX_regs_ar;
}

void IMX::set_test_regs(QByteArray reg_values) {


    int address[17]  = {0x438,0x439,0x43A,0x43C,0x43D,0x43E,0x43F,0x440,0x441,0x444,0x445,0x446,0x447,0x448,0x449,0x454,0x455};

    for(int i=0; i < 17; i++) {
        this->Cl_camera->send_TC(this->base_ad | address[i], reg_values.at(i));

     }


}


void IMX::configFREQ(quint8 DDR_mode_disable){
    if (cam_type==0)
      this->Cl_camera->send_TC(this->base_ad | IMX_CONF_FREQ[this->cam_type],  DDR_mode_disable);
    else {
        if (DDR_mode_disable)
            this->Cl_camera->send_TC(this->base_ad | IMX_CONF_FREQ[this->cam_type],  0xA3);
        else {
             this->Cl_camera->send_TC(this->base_ad | IMX_CONF_FREQ[this->cam_type],  0x93);
        }
    }

}

double IMX::Get_IMX_Bandwidth(quint8 nb_chan, quint8 nb_bits){

    double Imx_Mps;
    if (cam_type == 0){
    if (nb_chan == 16)
        Imx_Mps = 9504;
    else if (nb_chan == 8)
        Imx_Mps = 4752;
    else // 4
        Imx_Mps = 2376;
    }
    else {
        Imx_Mps = 2376;
    }
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

double IMX::Get_FPS( quint32 height, double imx_in_freq, quint8 nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable){

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



quint16 IMX::GetHMAX(quint8   nb_chan, quint8 nb_bits_sensor, quint8 DDR_mode_disable){

    quint16 HMAX;
    if (this->cam_type == 0){ // cinema
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
    }
    else {
        if (DDR_mode_disable == 0) {
           if (nb_bits_sensor == 8) {
               HMAX = 0x204;
           }else if (nb_bits_sensor == 10){
               HMAX = 0x22B;
           }else{ // nb_bit =
               HMAX = 0x3CC;
           }
       }else{
            if (nb_bits_sensor == 8) {
                HMAX = 0x2F6;
            }else if (nb_bits_sensor == 10){
                HMAX = 0x3AB;
            }else{ // nb_bit =
                HMAX = 0x45C;
            }
        }
   }


return HMAX;
}

double IMX::Get_exposure_MAX(quint8 imx_in_freq, quint16 height, quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable){

    double H_period = Get_H_period_IMX(imx_in_freq, nb_bits_sensor, nb_chan,  DDR_mode_disable);

    // double H_period = this->imx->Get_H_period_IMX(quint8(this->imx_freq_div->currentIndex()), nb_bits_sensor,  this->nb_chan,  this->DDR_mode_disable->isChecked());

    double exposure_MAX = (H_period * (height+VB1[cam_type]+frameInfo[cam_type] -20)+ 14.26)/1000;

    // qDebug().noquote() << "H_period     = " + QString::number(H_period) + " us";
    // qDebug().noquote() << "exposure MAX = " + QString::number(exposure_MAX) + " ms";

    return exposure_MAX;
}




double IMX::Get_H_period_IMX(quint8 imx_in_freq,  quint8 nb_bits_sensor, quint8 nb_chan, quint8 DDR_mode_disable){

    double INCK_PerPeriod;
   if (this->cam_type == 0){
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
   }
   else { // caspex IR
    if (DDR_mode_disable == 1) {
        if (nb_bits_sensor == 8 ){
                INCK_PerPeriod = 758.0;
        }
        else if (nb_bits_sensor == 10){

               INCK_PerPeriod = 940.0;
        }
        else {
               INCK_PerPeriod = 116.0;
        }

    }else {
        if (nb_bits_sensor == 8 ){
                INCK_PerPeriod = 512.0;
        }
        else if (nb_bits_sensor == 10){

               INCK_PerPeriod = 555;
        }
        else {
               INCK_PerPeriod = 972;
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
quint32 IMX::exposure_f_to_int(double exposure_time, double IMX_in_freq, quint8 fot_length) {
    double clk_per = 1 / IMX_in_freq;

    //qDebug () << "IMX_in_freq = " + QString::number(IMX_in_freq) + "MHz";
    //qDebug () << "clk_per = " + QString::number(clk_per) + "us";

    qint32 exp_time_int = static_cast<qint32>((exposure_time / (129.0*clk_per) - 0.43 * fot_length)); // a modif

    if (exp_time_int < 0) {
        return 0;
    }

    return static_cast<quint32>(exp_time_int);
}
