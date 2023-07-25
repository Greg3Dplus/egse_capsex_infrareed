#include "CLdialog.h"


spwdialog::spwdialog(quint8 cam_fw_type) : QDialog()
{
    this->mainLayout = new QGridLayout();

    QLabel *spw_port_lbl = new QLabel ("SPW Port");
    this->mainLayout->addWidget(spw_port_lbl, 0, 0);

    this->SPW_port = new QSpinBox();
    this->mainLayout->addWidget(this->SPW_port, 0, 1, 1, 2);
    this->SPW_port->setMinimum(1);
    this->SPW_port->setMaximum(2);

    QLabel *nb_bytes_lbl = new QLabel ("Nb Bytes per pixel");
    this->mainLayout->addWidget(nb_bytes_lbl, 1, 0);

    this->nb_bytes_per_pixel = new QSpinBox();
    this->mainLayout->addWidget(this->nb_bytes_per_pixel, 1, 1, 1, 2);
    this->nb_bytes_per_pixel->setMinimum(2);
    this->nb_bytes_per_pixel->setMaximum(2);
    if (cam_fw_type == CAM_FW_CO2M || cam_fw_type == CAM_FW_CINEMA) {
        this->nb_bytes_per_pixel->setMaximum(3);
    }

    this->TM = new QGroupBox("SpaceWire Transmit Clock");
    this->mainLayout->addWidget(this->TM, 2, 0, 1, 3);

    this->TMLayout = new QGridLayout();
    this->TM->setLayout(this->TMLayout);

    QLabel *mult = new QLabel ("Multiplier");
    this->TMLayout->addWidget(mult, 0, 0);
    QLabel *div  = new QLabel ("Divisor");
    this->TMLayout->addWidget(div, 1, 0);

    this->SPW_div = new QSpinBox();
    this->SPW_div->setMinimum(2);
    this->SPW_div->setMaximum(65536);
    this->SPW_div->setValue(2);
    connect(this->SPW_div,SIGNAL(valueChanged(int)),this,SLOT(on_SPW_valueChanged()));
    this->TMLayout->addWidget(this->SPW_div, 1, 1);

    this->SPW_mult = new QSpinBox();
    this->SPW_mult->setMinimum(2);
    this->SPW_mult->setMaximum(65536);
    this->SPW_mult->setValue(8);
    connect(this->SPW_mult,SIGNAL(valueChanged(int)),this,SLOT(on_SPW_valueChanged()));
    this->TMLayout->addWidget(this->SPW_mult, 0, 1);

    this->linkspeed = new QLabel("Current Link TX Speed");
    this->mainLayout->addWidget(this->linkspeed, 3, 0, 1, 3);

    this->ok = new QPushButton("OK");
    this->mainLayout->addWidget(this->ok, 4, 1);

    this->cancel = new QPushButton("Cancel");
    this->mainLayout->addWidget(this->cancel, 4, 2);

    connect(this->ok,SIGNAL(clicked()),this,SLOT(on_OK_clicked()));
    connect(this->cancel,SIGNAL(clicked()),this,SLOT(on_cancel_clicked()));

    setLayout(this->mainLayout);
    setWindowTitle("SpaceWire");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->UpdateInfo();

}

spwdialog::~spwdialog()
{
}

void spwdialog::on_OK_clicked()
{
    close();
}

void spwdialog::on_cancel_clicked()
{
    this->nb_bytes_per_pixel->setValue(this->init_params.nb_bytes_per_pixel);
    this->SPW_port->setValue(this->init_params.port_num);
    this->SPW_mult->setValue(this->init_params.mult);
    this->SPW_div->setValue(this->init_params.div);

    close();
}

SPW_params spwdialog::get_SPW_params()
{
    //qDebug().noquote()  << "Getting SPW parameters";
    SPW_params spw_param;
    spw_param.nb_bytes_per_pixel = static_cast<quint8>(this->nb_bytes_per_pixel->value());
    spw_param.port_num = static_cast<quint8>(this->SPW_port->value());
    spw_param.mult = static_cast<quint16>(this->SPW_mult->value());
    spw_param.div = static_cast<quint16>(this->SPW_div->value());

    return spw_param;
}

void spwdialog::set_SPW_params(quint8 port_num, quint16 mult, quint16 div, quint8 nb_bytes_per_pixel)
{
    this->nb_bytes_per_pixel->setValue(nb_bytes_per_pixel);
    this->SPW_port->setValue(port_num);
    this->SPW_mult->setValue(mult);
    this->SPW_div->setValue(div);

    this->init_params.mult = mult;
    this->init_params.div = div;
}


void spwdialog::on_SPW_valueChanged()
{
    this->UpdateInfo();
}

void spwdialog::UpdateInfo(void)
{

    double linkspeed_val = (100.0*this->SPW_mult->value()/this->SPW_div->value())*2.0;

    this->linkspeed->setText("Current Link Tx Speed : " + QString::number(linkspeed_val,0,2) + " Mbits/s");

    if(linkspeed_val> 200.0){
        this->linkspeed->setStyleSheet("color: red");
        this->ok->setDisabled(true);
    }
    else {
        this->linkspeed->setStyleSheet("");
        this->ok->setDisabled(false);
    }
}
