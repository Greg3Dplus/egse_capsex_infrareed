#include "aboutwindow.h"

aboutWindow::aboutWindow(quint32 egse_version) : QDialog()
{
    this->mainLayout = new QGridLayout();
    this->setFixedSize(175,115);

    this->fpga_version_lbl = new QLabel();
    this->mainLayout->addWidget(fpga_version_lbl, 2, 0, Qt::AlignCenter);

    QLabel *egse_version_lbl = new QLabel("EGSE Version : " + QString::number(egse_version));
    this->mainLayout->addWidget(egse_version_lbl, 1, 0 ,Qt::AlignCenter);

    QPixmap Logo(":/res/img/logo.png");
    QLabel* logo_lbl = new QLabel();
    logo_lbl->setMaximumWidth(50);
    logo_lbl->setPixmap(Logo.scaled(logo_lbl->width(), logo_lbl->height(), Qt::KeepAspectRatio));
    logo_lbl->setAlignment(Qt::AlignCenter);
    this->mainLayout->addWidget(logo_lbl, 0, 0, Qt::AlignCenter);

    this->ok = new QPushButton("OK");
    this->mainLayout->addWidget(this->ok, 5, 0, Qt::AlignCenter);

    connect(this->ok,SIGNAL(clicked()),this,SLOT(on_OK_clicked()));

    setLayout(this->mainLayout);
    setWindowTitle("About");

    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

aboutWindow::~aboutWindow()
{
}

void aboutWindow::set_fpga_version(QString project_name, quint32 fpga_version) {
    fpga_version_lbl->setText(project_name + " Version : " + QString::number(fpga_version));
}

void aboutWindow::on_OK_clicked()
{
    close();
}
