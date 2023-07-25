#include "test_mode_configuration.h"
#include "print_data.h"

Test_mode_configuration::Test_mode_configuration(QByteArray regs) : QDialog()
{
    QStringList names = {"PGMODE & PGCLKEN & PGTHRU & PGREGEN", "PGHPOS[7:0]", "PGHPOS[12:8]","PGVPOS[7:0]", "PGVPOS[11:8]","PGHSTEP[7:0]", "PGVSTEP[7:0]",
                        "PGHPNUM[7:0]", "PGVPNUM[7:0]","PGDATA1[7:0]", "PGDATA1[12:8]","PGDATA2[7:0]", "PGDATA2[12:8]","COLORWIDTH[7:0]","PGHGSTEP",
                        "BLKLEVEL[7:0]", "BLKLEVEL[11:8]"};

//   int address = {0x438,0x439,0x43A,0x43C,0x43D,0x43E,0x43F,0x440,0x441,0x444,0x445,0x446,0x447,0x448,0x449,0x454,0x455};



    this->mainLayout = new QGridLayout();

    QGridLayout *RegLayout = new QGridLayout();

    QWidget *test = new QWidget();
    test->setLayout(RegLayout);

    this->scrollArea = new QScrollArea();
    this->scrollArea->setWidget(test);
    this->scrollArea->setWidgetResizable(true);

    this->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    this->mainLayout->addWidget(this->scrollArea, 0, 0, 1, 3);

    for(int i=0; i < 17; i++)
    {
        this->reg_names.append(new QLabel("a" ));
        RegLayout->addWidget(reg_names.at(i), i, 0);

        reg_vals.append(new QSpinBox());
        reg_vals.at(i)->setMinimum(0);
        reg_vals.at(i)->setMaximum(255);

        RegLayout->addWidget(reg_vals.at(i), i, 1);

        reg_names.at(i)->setText("Reg  :" + names.at(i));
        reg_vals.at(i)->setValue(static_cast<quint8>(regs.at(i)));

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


Test_mode_configuration::~Test_mode_configuration()
{
}

void Test_mode_configuration::on_buttonBox_canceled()
{
    close();
}

void Test_mode_configuration::on_buttonBox_accepted()
{
    QByteArray IMX_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 17; i++) {
        IMX_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));

    }
    //  print_data_Bytes(IMX_reg_values);
    //qDebug() << "End of on_ButtonBox_accepted";
    close();
}

QByteArray Test_mode_configuration::get_test_mode_regs_values()
{
    QByteArray IMX_reg_values;

    // Get all the register values from QSpinBox
    for(int i=0; i < 17; i++) {
        IMX_reg_values.append(static_cast<qint8>(this->reg_vals.at(i)->value()));
    }

    // print_data_Bytes(IMX_reg_values);
    //qDebug() << "End of get_imx_regs_values";

    return IMX_reg_values;
}


