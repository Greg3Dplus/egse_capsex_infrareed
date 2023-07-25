#include "sendcmddialog.h"
#include "print_data.h"

sendcmddialog::sendcmddialog() : QDialog()
{
    QString str("HH ");

    this->mainLayout  = new QGridLayout();
    this->readLayout  = new QGridLayout();
    this->writeLayout = new QGridLayout();
    this->TMLayout    = new QGridLayout();
    this->TCLayout    = new QGridLayout();

    this->TMbox = new QGroupBox("Telemetry (Read data)");
    this->TMbox->setLayout(this->TMLayout);
    this->mainLayout->addWidget(this->TMbox, 0, 3);

    this->TCbox = new QGroupBox("Telecommand (Write data)");
    this->TCbox->setLayout(this->TCLayout);
    this->mainLayout->addWidget(this->TCbox, 0, 0);

    QLabel *start_add_TC = new QLabel("Starting address");
    this->TCLayout->addWidget(start_add_TC, 0, 0);

    QLabel *transmitpkts_label = new QLabel("Please enter bytes to transmit (in hexadecimal format) :");
    this->mainLayout->addWidget(transmitpkts_label, 2, 0, 1, 2);


    this->TC_addr = new QSpinBox();
    this->TC_addr->setPrefix("0x");
    this->TC_addr->setDisplayIntegerBase(16);
    this->TC_addr->setMinimum(0);
    this->TC_addr->setMaximum(429496729);
    this->TC_addr->setValue(0x800);
    QObject::connect(this->TC_addr, SIGNAL(valueChanged(int)), this, SLOT(on_TC_addr_valueChanged(int)));
    this->TCLayout->addWidget(this->TC_addr, 0, 1);

    this->TC_pushButton = new QPushButton("Send TC");
    QObject::connect(this->TC_pushButton, SIGNAL(clicked()), this, SLOT(on_TC_pushButton_clicked()));
    this->mainLayout->addWidget(this->TC_pushButton, 0, 1);


    QLabel *receveidpkts_label = new QLabel("Received bytes :");
    this->mainLayout->addWidget(receveidpkts_label, 1, 3);

    this->receivedpkts = new QPlainTextEdit();
    this->mainLayout->addWidget(this->receivedpkts, 2, 3, 5, 2);


    QLabel *start_add_TM = new QLabel("Starting address");
    this->TMLayout->addWidget(start_add_TM, 1, 0);

    QLabel *nb_byte = new QLabel("Number of bytes");
    this->TMLayout->addWidget(nb_byte, 0, 0);

    this->TM_addr = new QSpinBox();
    this->TM_addr->setPrefix("0x");
    this->TM_addr->setDisplayIntegerBase(16);
    this->TM_addr->setMinimum(0);
    this->TM_addr->setMaximum(429496729);
    this->TM_addr->setValue(0x800);
    this->TMLayout->addWidget(this->TM_addr, 1, 1);
    QObject::connect(this->TM_addr, SIGNAL(valueChanged(int)), this, SLOT(on_TM_addr_valueChanged(int)));

    this->TM_nbdata = new QSpinBox();
    this->TM_nbdata->setMinimum(1);
    this->TM_nbdata->setMaximum(128);
    this->TM_nbdata->setValue(128);
    this->TMLayout->addWidget(this->TM_nbdata, 0, 1);
    QObject::connect(this->TM_nbdata, SIGNAL(valueChanged(int)), this, SLOT(on_TM_nbdata_valueChanged(int)));

    this->TM_pushButton = new QPushButton("Send TM");
    QObject::connect(this->TM_pushButton, SIGNAL(clicked()), this, SLOT(on_TM_pushButton_clicked()));
    this->mainLayout->addWidget(this->TM_pushButton, 0, 4);


    this->TC_status = new QLabel();
    this->mainLayout->addWidget(this->TC_status, 4, 0);

    this->transmitpkts = new QLineEdit();
    this->mainLayout->addWidget(this->transmitpkts, 3, 0, 1 ,2);
    QObject::connect(this->transmitpkts, SIGNAL(textChanged(const QString &)), this, SLOT(on_transmitpkts_textChanged()));

    this->transmitpkts->setInputMask(str.repeated(128)); // Limited to 128 Bytes
    m_return.installOn(this->transmitpkts);

    TC_params.addr    = static_cast<quint32>(this->TC_addr->value());
    TM_params.nb_data = static_cast<quint8>(this->TM_nbdata->value());
    TM_params.addr    = static_cast<quint32>(this->TM_addr->value());


    QObject::connect(this, SIGNAL(finished(int)), this, SLOT(on_sendcmddialog_finished()));
    setLayout(this->mainLayout);
    setWindowTitle("TM/TC Configuration Window");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

sendcmddialog::~sendcmddialog()
{
}


TMTC_params sendcmddialog::get_TM_params() const
{
    return this->TM_params;
}

TMTC_params sendcmddialog::get_TC_params() const
{
    return this->TC_params;
}

void sendcmddialog::set_TM_params(const QString &received_pkt, bool reset)
{
    this->receivedpkts->appendPlainText(received_pkt);

    if (reset == true) {
        this->receivedpkts->clear();
    }
}


void sendcmddialog::on_TM_pushButton_clicked()
{
    TM_params.data_pkts = "";
    this->receivedpkts->clear();
    emit TM_clicked();
}

void sendcmddialog::on_sendcmddialog_finished(void)
{
    this->transmitpkts->clear();
    emit TM_closed();
}

void sendcmddialog::on_TC_pushButton_clicked()
{
    if (TC_params.data_pkts.size() % 2 != 0){
        QMessageBox msgBox;
        QString msg = "Invalid entry : Number of characters entered shall be even.";
        msgBox.setText(msg);
        msgBox.exec();
    }
    else {
        emit TC_clicked();
    }
}

void sendcmddialog::set_TC_params(const QString &TC_txt, bool reset)
{
    this->TC_status->setText(TC_txt);

    if (reset) {
        this->TC_status->clear();
    }
}

void sendcmddialog::on_TC_addr_valueChanged(int value)
{
    TC_params.addr = static_cast<quint32>(value);
}

void sendcmddialog::on_TM_nbdata_valueChanged(int value)
{
    TM_params.nb_data = static_cast<quint8>(value);
}

void sendcmddialog::on_TM_addr_valueChanged(int value)
{
    TM_params.addr = static_cast<quint32>(value);
}

void sendcmddialog::on_transmitpkts_textChanged()
{
    // Remove whitespaces from user input
    QString str = this->transmitpkts->text().simplified();
    TC_params.data_pkts = str.replace( " ", "" );
    // TC_params.data_pkts = "19181716151413121110090807060504030201";
}
