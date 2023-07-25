#include "cmv_read_registers.h"
#include "print_data.h"

CMV_read_registers::CMV_read_registers(QByteArray cmv_regs) : QDialog()
{
    QStringList names = {"Reserved", "Number_lines(7:0)", "Number_lines(15:8)", "Start1(7:0)", "Start1(15:8)", "Start2(7:0)", "Start2(15:8)", "Start3(7:0)",
                         "Start3(15:8)", "Start4(7:0)", "Start4(15:8)", "Start5(7:0)", "Start5(15:8)", "Start6(7:0)", "Start6(15:8)", "Start7(7:0)",
                         "Start7(15:8)", "Start8(7:0)", "Start8(15:8)", "Number_lines1(7:0)", "Number_lines1(15:8)", "Number_lines2(7:0)", "Number_lines2(15:8)", "Number_lines3(7:0)",
                         "Number_lines3(15:8)", "Number_lines4(7:0)", "Number_lines4(15:8)", "Number_lines5(7:0)", "Number_lines5(15:8)", "Number_lines6(7:0)", "Number_lines6(15:8)", "Number_lines7(7:0)",
                         "Number_lines7(15:8)", "Number_lines8(7:0)", "Number_lines8(15:8)", "Sub_s(7:0)", "Sub_s(15:8)", "Sub_a(7:0)", "Sub_a(15:8)", "mono",
                         "Image_flipping(1:0)", "Exp", "Exp_time(7:0)", "Exp_time(15:8)", "Exp_time(23:16)", "Exp_step(7:0)", "Exp_step(15:8)", "Exp_step(23:16)",
                         "Exp_kp1(7:0)", "Exp_kp1(15:8)", "Exp_kp1(23:16)", "Exp_kp2(7:0)", "Exp_kp2(15:8)", "Exp_kp2(23:16)", "Nr_slopes(1:0)", "Exp_seq(7:0)",
                         "Exp_time2(7:0)", "Exp_time2(15:8)", "Exp_time2(23:16)", "Exp_step2(7:0)", "Exp_step2(15:8)", "Exp_step2(23:16)", "Reserved", "Reserved",
                         "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Exp2_seq(7:0)", "Number_frames(7:0)", "Number_frames(15:8)",
                         "Output_mode(1:0)", "fot_length", "i_lvds_rec(3:0)", "Reserved", "Reserved", "ADC_calib", "Training_pattern(7:0)", "Training_pattern(11:8)",
                         "Channel_en(7:0)", "Channel_en(15:8)", "Channel_en(18:16)", "i_lvds(3:0)", "I_col(3:0)", "I_col_prech(3:0)", "Reserved", "I_amp(3:0)",
                         "Wtf_I1(6:0)", "Vlow2(6:0)", "Vlow3(6:0)", "Vres_low(6:0)", "Reserved", "Reserved", "V_prech(6:0)", "V_ref(6:0)",
                         "Reserved", "Reserved", "Vramp1(6:0)", "Vramp2(6:0)", "Offset(7:0)", "Offset(13:8)", "PGA_Gain(1:0)", "ADC_Gain(7:0)",
                         "Reserved", "Reserved", "Reserved", "Reserved", "T_dig1(3:0)", "T_dig2(3:0)", "Reserved", "Bit_mode",
                         "ADC_resolution(1:0)", "pll_enable", "PLL_IN_FRE(1:0)", "pll_bypass", "PLL_DIV(3:0)", "PLL_load(7:0)", "Dummy(7:0)", "Reserved",
                         "Reserved", "PGA_Gain(2)", "Reserved", "V_blacksun(5:0)", "Reserved", "Version", "Temp(7:0)", "Temp(15:8)"};

    //qDebug() << "Size of array = " + QString::number(cmv_regs.size(), 10);
    //qDebug() << "Size of names = " + QString::number(names.size(), 10);

    this->mainLayout = new QGridLayout();

    QGridLayout *RegLayout = new QGridLayout();

    QWidget *test = new QWidget();
    test->setLayout(RegLayout);

    this->scrollArea = new QScrollArea();
    this->scrollArea->setWidget(test);
    this->scrollArea->setWidgetResizable(true);

    this->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    this->mainLayout->addWidget(this->scrollArea, 0, 0, 1, 3);


    for(int i=0; i < 128; i++)
    {
        this->reg_names.append(new QLabel("a"));
        RegLayout->addWidget(reg_names.at(i), i, 0);

        reg_vals.append(new QSpinBox());
        reg_vals.at(i)->setMinimum(0);
        reg_vals.at(i)->setMaximum(255);

        if ((i >= 100 && i <= 101) || i == 103) {
            reg_vals.at(i)->setEnabled(false);
        }
        RegLayout->addWidget(reg_vals.at(i), i, 1);

        reg_names.at(i)->setText("Reg " + QString::number(i, 10) + ": " + names.at(i));
        reg_vals.at(i)->setValue(static_cast<quint8>(cmv_regs.at(i)));

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
    setWindowTitle("CMV registers list");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}


CMV_read_registers::~CMV_read_registers()
{
}

void CMV_read_registers::on_buttonBox_canceled()
{
    close();
}

void CMV_read_registers::on_buttonBox_accepted()
{
    QByteArray cmv_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 128; i++) {
        cmv_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));

    }
    //  print_data_Bytes(cmv_reg_values);
    //qDebug() << "End of on_ButtonBox_accepted";
    close();
}

QByteArray CMV_read_registers::get_cmv_regs_values()
{
    QByteArray cmv_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 128; i++) {
        cmv_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));
    }

    // print_data_Bytes(cmv_reg_values);
    //qDebug() << "End of get_cmv_regs_values";

    return cmv_reg_values;
}


