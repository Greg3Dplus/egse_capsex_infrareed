#include "imx_read_registers.h"
#include "print_data.h"

IMX_read_registers::IMX_read_registers(QByteArray imx_regs) : QDialog()
{
    QStringList names = {"STANDBY[0]","STBLVDS[3:0]","REGHOLD[0]","XMSTA[0]","TRIGEN[0]","ADBIT[1:0]","HDMODE[0] & WINMODE[3:0]","HREVERSE[0] & VREVERSE[0]",
                         "VMAX[7:0]","VMAX[15:8]","VMAX[19:16]","HMAX[7:0]","HMAX[15:8]","ODBIT[1:0]","FREQ[1:0]","OPORTSEL[3:0]","TOUT1SEL[1:0] & TOUT2SEL[1:0]",
                         "TRIG_TOUTx_SEL",
                         "SYNCSEL[1:0]","PULSE1_EN","PULSE1_UP[7:0]","PULSE1_UP[15:8]","PULSE1_UP[19:16]","PULSE1_DN[7:0]","PULSE1_DN[15:8]","PULSE1_DN[19:16]",
                         "PULSE2_EN","PULSE2_UP[7:0]","PULSE2_UP[15:8]","PULSE2_UP[19:16]","PULSE2_DN[7:0]","PULSE2_DN[15:8]","PULSE2_DN[19:16]","INCKSEL0[7:0]",
                         "INCKSEL1[7:0]","INCKSEL2[7:0]","INCKSEL3[7:0]","SHS[7:0]","SHS[15:8]","SHS[19:16]","VINT_EN[0]","LOWLAGTRG[0]","OVERLAP_ROI_EN[1]","GAIN[7:0]",
                         "GAIN[8]","GAINDLY[7:0]","BLKLEVEL[7:0]","BLKLEVEL[11:8]","VOPB_VBLK_HWIDTH[7:0]", "VOPB_VBLK_HWIDTH[12:8]",  "FINFO_HWIDTH[7:0]",
                         "FINFO_HWIDTH[12:8]","FID0_ROIHxON (1,2,3,4)","FID0_ROIHxON (5,6,7,8)","FID0_ROIPH1[7:0]","FID0_ROIPH1[12:8]","FID0_ROIPV1[7:0]","FID0_ROIPV1[11:8]",
                        "FID0_ROIWH1[7:0]","FID0_ROIWH1[12:8]","FID0_ROIWV1[7:0]","FID0_ROIWV1[11:8]","FID0_ROIPH2[7:0]","FID0_ROIPH2[12:8]","FID0_ROIPV2[7:0]","FID0_ROIPV2[11:8]",
                         "FID0_ROIWH2[7:0]","FID0_ROIWH2[12:8]","FID0_ROIWV2[7:0]","FID0_ROIWV2[11:8]","FID0_ROIPH3[7:0]","FID0_ROIPH3[12:8]","FID0_ROIPV3[7:0]","FID0_ROIPV3[11:8]",
                         "FID0_ROIWH3[7:0]","FID0_ROIWH3[12:8]","FID0_ROIWV3[7:0]","FID0_ROIWV3[11:8]","FID0_ROIPH4[7:0]","FID0_ROIPH4[12:8]","FID0_ROIPV4[7:0]","FID0_ROIPV4[11:8]",
                         "FID0_ROIWH4[7:0]","FID0_ROIWH4[12:8]","FID0_ROIWV4[7:0]","FID0_ROIWV4[11:8]","FID0_ROIPH5[7:0]","FID0_ROIPH5[12:8]","FID0_ROIPV5[7:0]","FID0_ROIPV5[11:8]",
                         "FID0_ROIWH5[7:0]","FID0_ROIWH5[12:8]","FID0_ROIWV5[7:0]","FID0_ROIWV5[11:8]","FID0_ROIPH6[7:0]","FID0_ROIPH6[12:8]","FID0_ROIPV6[7:0]","FID0_ROIPV6[11:8]",
                         "FID0_ROIWH6[7:0]","FID0_ROIWH6[12:8]","FID0_ROIWV6[7:0]","FID0_ROIWV6[11:8]","FID0_ROIPH7[7:0]","FID0_ROIPH7[12:8]","FID0_ROIPV7[7:0]","FID0_ROIPV7[11:8]",
                         "FID0_ROIWH7[7:0]","FID0_ROIWH7[12:8]","FID0_ROIWV7[7:0]","FID0_ROIWV7[11:8]","FID0_ROIPH8[7:0]","FID0_ROIPH8[12:8]","FID0_ROIPV8[7:0]","FID0_ROIPV8[11:8]",
                         "FID0_ROIWH8[7:0]","FID0_ROIWH8[12:8]","FID0_ROIWV8[7:0]","FID0_ROIWV8[11:8]"};

    int address[118]   = {0x200,0x205,0x208,0x20A,0x20B,0X20C,0x20D,0x20E,0x210,0x211,0x212,0x214,0x215,0x216,0x21B,0x21C,
                          0x226,0x229,0x236,0x26D,0x270,0X271,0x272,0x274,0x275,0x276,0x279,0x27C,0x27D,0x27E,0x280,0x281,
                          0x282,0x289,0x28A,0x28B,0x28C,0x28D,0x28E,0x28F,0x2AA,0x2AE,0x2B0,0x404,0x405,0x412,0x454,0x455,
                          0x474,0x475,0x476,0x477,0x500,0x501,0x510,0x511,0x512,0x513,0x514,0x515,0x516,0x517,0x518,0x519,
                          0x51A,0x51B,0x51C,0x51D,0x51E,0x51F,0x520,0x521,0x522,0x523,0x524,0x525,0x526,0x527,0x528,0x529,
                          0x52A,0x52B,0x52C,0x52D,0x52E,0x52F,0x530,0x531,0x532,0x533,0x534,0x535,0x536,0x537,0x538,0x539,
                          0x53A,0x53B,0x53C,0x53D,0x53E,0x53F,0x540,0x541,0x542,0x543,0x544,0x545,0x546,0x547,0x548,0x549,
                          0x54A,0x54B,0x54C,0x54D,0x54E,0x54F};

    this->mainLayout = new QGridLayout();

    QGridLayout *RegLayout = new QGridLayout();

    QWidget *test = new QWidget();
    test->setLayout(RegLayout);

    this->scrollArea = new QScrollArea();
    this->scrollArea->setWidget(test);
    this->scrollArea->setWidgetResizable(true);

    this->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    this->mainLayout->addWidget(this->scrollArea, 0, 0, 1, 3);

    for(int i=0; i < 118; i++)
    {
        this->reg_names.append(new QLabel("a" ));
        RegLayout->addWidget(reg_names.at(i), i, 0);

        reg_vals.append(new QSpinBox());
        reg_vals.at(i)->setMinimum(0);
        reg_vals.at(i)->setMaximum(255);

        reg_vals.at(i)->setPrefix("0x");
        reg_vals.at(i)->setDisplayIntegerBase(16);

        RegLayout->addWidget(reg_vals.at(i), i, 1);

        reg_names.at(i)->setText("Chip:" +  QString::number((address[i] & 0xF00)>>8,16)+  "  Reg: 0x" + QString::number((address[i] & 0x0FF),16) +"   " + names.at(i));
        reg_vals.at(i)->setValue(static_cast<quint8>(imx_regs.at(i)));

        connect(reg_vals.at(i), QOverload<int>::of(&QSpinBox::valueChanged), [=](int){
            reg_vals.at(i)->setStyleSheet("QSpinBox { background-color: red; }");
        });

    }

    QPushButton *ok = new QPushButton("Ok");
    this->mainLayout->addWidget(ok, 2, 1);
    QObject::connect(ok,     SIGNAL(clicked()),        this, SLOT(on_buttonBox_accepted()));

    QPushButton *cancel = new QPushButton("Cancel");
    this->mainLayout->addWidget(cancel, 2, 2);
    QObject::connect(cancel,     SIGNAL(clicked()),        this, SLOT(on_buttonBox_canceled()));


    setLayout(this->mainLayout);
    setWindowTitle("IMX registers list");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}


IMX_read_registers::~IMX_read_registers()
{
}

void IMX_read_registers::on_buttonBox_canceled()
{
    close();
}

void IMX_read_registers::on_buttonBox_accepted()
{
    QByteArray IMX_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 118; i++) {
        IMX_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));

    }
    //  print_data_Bytes(IMX_reg_values);
    //qDebug() << "End of on_ButtonBox_accepted";
    close();
}

QByteArray IMX_read_registers::get_imx_regs_values()
{
    QByteArray IMX_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 118; i++) {
        IMX_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));
    }

    // print_data_Bytes(IMX_reg_values);
    // qDebug() << "End of get_imx_regs_values";

    return IMX_reg_values;
}


