#include "CLdialog.h"


CLdialog::CLdialog(quint8 TapNumber) : QDialog()
{
    this->TapNumber = TapNumber;

    this->mainLayout = new QGridLayout();

    this->tap_number = new QComboBox();
    QLabel *tap_number_lbl = new QLabel("Tap number");

    this->mainLayout->addWidget(tap_number_lbl, 2, 0);
    this->mainLayout->addWidget(this->tap_number, 2, 1);

    this->tap_number->addItem("1");
    this->tap_number->addItem("2");
//    this->tap_number->addItem("4");
//    this->tap_number->addItem("8");
    quint8 InitIndex = quint8 (log2(TapNumber));
    this->tap_number->setCurrentIndex(InitIndex);

    QLabel *config_port_lbl = new QLabel ("CameraLink configuration:");
    this->mainLayout->addWidget(config_port_lbl, 0, 0);
    QLabel *cl_freq_lbl = new QLabel ("CameraLink frequency : 80 MHz");
    this->mainLayout->addWidget(cl_freq_lbl, 1, 0);
    QLabel *configExplanation = new QLabel ("Base: 1 Taps; Medium: 2 Taps");
    this->mainLayout->addWidget(configExplanation, 4, 0);
    QLabel *cl_serial_baud_lbl = new QLabel ("Current Serial interface baud rate: 115200");
    this->mainLayout->addWidget(cl_serial_baud_lbl, 5, 0);

//    QObject::connect(this->tap_number,     SIGNAL(currentTextChanged(QString)), this, SLOT(on_Tap_number_change()));

    this->ok = new QPushButton("OK");
    this->mainLayout->addWidget(this->ok, 5, 1);

    this->cancel = new QPushButton("Cancel");
    this->mainLayout->addWidget(this->cancel, 5, 2);

    connect(this->ok,SIGNAL(clicked()),this,SLOT(on_OK_clicked()));
    connect(this->cancel,SIGNAL(clicked()),this,SLOT(on_cancel_clicked()));

    setLayout(this->mainLayout);
    setWindowTitle("CameraLink configuration");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);



}

CLdialog::~CLdialog()
{
}

void CLdialog::on_OK_clicked()
{


    close();
}

void CLdialog::on_cancel_clicked()
{

    this->tap_number->setCurrentIndex(quint8(log2(this->TapNumber)));
    close();
}



CL_params CLdialog::Get_Cl_params(){
    CL_params cl_params;
    cl_params.TapNumber = static_cast<quint8>(pow(2,this->tap_number->currentIndex()));
    return cl_params;
}

///void CLdialog::Set_Cl_params( quint8 TapNumber){

//    this->index = quint8 (pow(2,TapNumber));
////     qDebug().noquote()  << "tap "+ QString::number(index,10);
////    this->SpinboxTest->setValue(index);
////     //this->tap_number->setCurrentIndex(index);
////    qDebug().noquote()  << "top";
////    QString CL_config;
////    if (TapNumber > 1){

////     CL_config = "CameraLink Selected configuration : Medium";

////    }else  if (TapNumber > 4){

////         CL_config = "CameraLink Selected configuration : Full";
////    }else
////         CL_config = "CameraLink Selected configuration : Base";
////     qDebug().noquote()  << CL_config;
////     this->config_port_lbl->setText(CL_config);
//}//




//void CLdialog::on_Tap_number_change(void){
//    QString CL_config =  updateInfo();
//     qDebug().noquote()  << CL_config;
//     this->config_port_lbl->setText(CL_config);
//}

//QString CLdialog::updateInfo(){

//    QString CL_config;
//    if (this->tap_number->currentIndex() > 1){

//     CL_config = "CameraLink Selected configuration : Medium";

//    }else  if (this->tap_number->currentIndex() > 4){

//         CL_config = "CameraLink Selected configuration : Full";
//    }else
//         CL_config = "CameraLink Selected configuration : Base";

//   return CL_config;
//}
