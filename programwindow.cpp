#include "programwindow.h"

ProgramWindow::ProgramWindow(quint8 cam_hw_type, QString file_type) : QDialog()
{
    this->cam_hw_type = cam_hw_type;
    this->file_type = file_type;

    this->mainLayout = new QGridLayout();

    this->browseExe = new QPushButton("Browse");
    this->mainLayout->addWidget(this->browseExe, 0, 3);

    this->browsePdb = new QPushButton("Browse");
    this->mainLayout->addWidget(this->browsePdb, 1, 3);

    this->program   = new QPushButton("Program FPGA");
    if (this->file_type == ".mcs") {
        this->program->setText("Program SPI NOR Flash");
    }
    this->mainLayout->addWidget(this->program, 2, 2);

    this->pathExe = new QLineEdit(this->get_exe_default_path());
    this->pathExe->setMinimumWidth(300);
    this->mainLayout->addWidget(this->pathExe, 0, 2);

    this->pathBitstream = new QLineEdit(this->get_default_bitstream());
    this->pathBitstream->setMinimumWidth(300);
    this->mainLayout->addWidget(this->pathBitstream, 1, 2);

    QObject::connect(this->browseExe, SIGNAL(clicked()), this, SLOT(on_browseExe_clicked()));
    QObject::connect(this->browsePdb, SIGNAL(clicked()), this, SLOT(on_browsePdb_clicked()));
    QObject::connect(this->program, SIGNAL(clicked()), this, SLOT(on_button_program_clicked()));

    this->program_sts = new QLabel();
    this->mainLayout->addWidget(this->program_sts, 2, 3);

    QStringList tool_name;
    tool_name.append("FLASHPRO");
    tool_name.append("FLASHPRO");
    tool_name.append("Vivado");
    tool_name.append("Vivado");

    QStringList extension_name;
    extension_name.append(".pdb");
    extension_name.append(".pdb");
    extension_name.append(this->file_type);
    extension_name.append(this->file_type);

    QLabel *exe_lbl = new QLabel("Path to " + tool_name.at(this->cam_hw_type) + " executable:");
    this->mainLayout->addWidget(exe_lbl, 0, 0);
    QLabel *pdb_lbl = new QLabel("Path to bistream file (*" + extension_name.at(this->cam_hw_type) +"):");
    this->mainLayout->addWidget(pdb_lbl, 1, 0);

    setLayout(this->mainLayout);
    setWindowTitle("Program FPGA");
    if (this->file_type == ".mcs") {
        setWindowTitle("Program SPI NOR Flash");
    }
    //Remove question mark from title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}

ProgramWindow::~ProgramWindow()
{
}

QString ProgramWindow::get_exe_default_path() {
    QStringList exe_default_path;
    if (this->cam_hw_type == CAM_HW_CO2M || this->cam_hw_type == CAM_HW_CINEMA) {
        return "C:/Xilinx/Vivado/2018.3/bin/vivado";
    }
    else {
        return "C:/Microsemi/Libero_SoC_v11.9/Designer/bin/flashpro.exe";
    }
}

QString ProgramWindow::get_default_bitstream() {
    QStringList exe_default_path;
    if (this->cam_hw_type == CAM_HW_CO2M) {
        return "L:/CAO/projets_CS/co2m/vivado_win_2/co2m.runs/impl_1/top_co2m" + this->file_type;  // bit or mcs
    }
    else if (this->cam_hw_type == CAM_HW_CINEMA) {
        qDebug() << "L:/CAO/projets_GP/cinema/design_flow/cinema.runs/impl_1/top_cinema" + this->file_type;
        return "L:/CAO/projets_GP/cinema/design_flow/cinema.runs/impl_1/top_cinema" + this->file_type;  // bit or mcs
    }
    else {
        return "./3DIPCC0746.pdb";
    }
}

QString ProgramWindow::get_exec_path() {
    return this->pathExe->text();
}

QString ProgramWindow::get_bitstream_path() {
    return this->pathBitstream->text();
}

void ProgramWindow::on_browseExe_clicked()
{

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Executable (*.exe);; All files (*.*)"));
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList fileNames;
    if(dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        this->pathExe->setText(fileNames.at(0));
        fileNames.removeAt(0);
    }
    else
        dialog.reject();

}

void ProgramWindow::on_button_program_clicked()
{

    QString project_dir_name = "flashpro_project";
    QString tcl_file_name = "script_program_FPGA.tcl";
    QFile outputFile(tcl_file_name);

    // Create new file
    if (outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream out(&outputFile);

        if (this->cam_hw_type == CAM_HW_3DCM734 || this->cam_hw_type == CAM_HW_3DCM739) {
            out << "new_project -name {3DIPCC0746} -location {./" + project_dir_name + "} -mode {single} -connect_programmers 1\n";
            out << "set_programming_file -file {" + this->get_bitstream_path() + "}\n";
            out << "set_programming_action -action {ERASE}\n";
            out << "set_programming_action -action {PROGRAM}\n";
            out << "run_selected_actions\n";
            out << "save_project\n";
            out << "close_project\n";
        }
        else if (this->cam_hw_type == CAM_HW_CO2M)  {
            if (this->file_type == ".bit") {
                out << "open_hw\n";
                out << "connect_hw_server\n";
                out << "open_hw_target\n";

                out << "current_hw_device [get_hw_devices xc7k160t_0]\n";
                out << "refresh_hw_device [lindex [get_hw_devices xc7k160t_0] 0]\n";
                out << "create_hw_cfgmem -hw_device [get_hw_devices xc7k160t_0] -mem_dev [lindex [get_cfgmem_parts {is25lp128-spi-x1_x2_x4}] 0]\n";

                out << "set_property PROGRAM.FILE {" + this->get_bitstream_path() + "} [get_hw_devices xc7k160t_0]\n";
                out << "set_property PROBES.FILE {L:/CAO/projets_CS/co2m/vivado_win_2/co2m.runs/impl_1/top_co2m.ltx} [get_hw_devices xc7k160t_0]\n";
                out << "set_property FULL_PROBES.FILE {L:/CAO/projets_CS/co2m/vivado_win_2/co2m.runs/impl_1/top_co2m.ltx} [get_hw_devices xc7k160t_0]\n";
                out << "program_hw_devices [get_hw_devices xc7k160t_0]\n";
            }
            else {
                out << "open_hw\n";
                out << "connect_hw_server\n";
                out << "open_hw_target\n";
                out << "current_hw_device [get_hw_devices xc7k160t_0]\n";
                out << "refresh_hw_device -update_hw_probes false [lindex [get_hw_devices xc7k160t_0] 0]\n";
                out << "create_hw_cfgmem -hw_device [lindex [get_hw_devices xc7k160t_0] 0] [lindex [get_cfgmem_parts {is25lp128-spi-x1_x2_x4}] 0]\n";

                out << "set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.FILES [list \"" + this->get_bitstream_path() + "\" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.PRM_FILE {} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";

                out << "create_hw_bitstream -hw_device [lindex [get_hw_devices xc7k160t_0] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "program_hw_devices [lindex [get_hw_devices xc7k160t_0] 0]\n";
                out << "refresh_hw_device [lindex [get_hw_devices xc7k160t_0] 0]\n";
                out << "program_hw_cfgmem -hw_cfgmem [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xc7k160t_0] 0]]\n";
                out << "boot_hw_device  [lindex [get_hw_devices xc7k160t_0] 0]\n";
             }
        }
        else { // CINEMA

            if (this->file_type == ".bit") {
                out << "open_hw\n";
                out << "connect_hw_server\n";
                out << "open_hw_target\n";

                out << "current_hw_device [get_hw_devices xcku040_0]\n";
                out << "refresh_hw_device [lindex [get_hw_devices xcku040_0] 0]\n";
                out << "create_hw_cfgmem -hw_device [get_hw_devices xcku040_0] -mem_dev [lindex [get_cfgmem_parts {is25lp128-spi-x1_x2_x4}] 0]\n";

                out << "set_property PROGRAM.FILE {" + this->get_bitstream_path() + "} [get_hw_devices xcku040_0]\n";
                out << "set_property PROBES.FILE {L:/CAO/projets_GP/cinema/design_flow/cinema.runs/impl_1/top_cinema.ltx} [get_hw_devices xcku040_0]\n";
                out << "set_property FULL_PROBES.FILE {L:/CAO/projets_GP/cinema/design_flow/cinema.runs/impl_1/top_cinema.ltx} [get_hw_devices xcku040_0]\n";
                out << "program_hw_devices [get_hw_devices xcku040_0]\n";
            }
            else {
                out << "open_hw\n";
                out << "connect_hw_server\n";
                out << "open_hw_target\n";
                out << "current_hw_device [get_hw_devices xcku040_0]\n";
                out << "refresh_hw_device -update_hw_probes false [lindex [get_hw_devices xcku040_0] 0]\n";
                out << "create_hw_cfgmem -hw_device [lindex [get_hw_devices xcku040_0] 0] [lindex [get_cfgmem_parts {is25lp128-spi-x1_x2_x4}] 0]\n";

                out << "set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.CHECKSUM  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.FILES [list \"" + this->get_bitstream_path() + "\" ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.PRM_FILE {} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";

                out << "create_hw_bitstream -hw_device [lindex [get_hw_devices xcku040_0] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "program_hw_devices [lindex [get_hw_devices xcku040_0] 0]\n";
                out << "refresh_hw_device [lindex [get_hw_devices xcku040_0] 0]\n";
                out << "program_hw_cfgmem -hw_cfgmem [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices xcku040_0] 0]]\n";
                out << "boot_hw_device  [lindex [get_hw_devices xcku040_0] 0]\n";
            }

        }

        outputFile.close();
    }
    else {
        qDebug () << "ERROR: File not opened";
    }

    QProcess exec;

    QDir dir(project_dir_name);
    dir.removeRecursively();

    bool error = false;

    if (this->cam_hw_type == CAM_HW_3DCM734 || this->cam_hw_type == CAM_HW_3DCM739) {
        QStringList Arguments = QStringList{
                QString::fromStdString("script:") + tcl_file_name,
                QString::fromStdString("logfile:3DIPCC0746.log"),
                QString::fromStdString("console_mode:brief")};

        exec.start(this->get_exec_path(), Arguments);

        exec.waitForStarted(-1);

        //qDebug().noquote()  << "STARTED";

        exec.waitForFinished(-1);

        //qDebug().noquote()  << "FINISHED";

        if ((exec.error() != QProcess::UnknownError) || (exec.exitStatus() != QProcess::NormalExit)) {           // Unknown error is the default value, which is returned when there is no error
            error = true;
        }
    }
    else {
        QString cmd = this->get_exec_path() + " -mode batch -source " + tcl_file_name;
        system(cmd.toUtf8());
    }

    if (error) {
        this->program_sts->setText("FPGA programming error!");
        if (this->file_type == ".mcs") {
            this->program_sts->setText("SPI programming error!");
        }
        this->program_sts->setStyleSheet("font-weight: bold; color: red");
    }
    else {
        this->program_sts->setText("FPGA programming done!");
        if (this->file_type == ".mcs") {
            this->program_sts->setText("SPI programming done!");
        }
        this->program_sts->setStyleSheet("font-weight: bold; color: green");
    }
}

void ProgramWindow::on_browsePdb_clicked()
{

    QStringList extension_name;
    extension_name.append(".pdb");
    extension_name.append(".pdb");
    extension_name.append(this->file_type);
    extension_name.append(this->file_type);

    this->program_sts->clear();

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Bitstream (*" + extension_name.at(this->cam_hw_type) + ");; All files (*.*)");
    dialog.setViewMode(QFileDialog::List);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    QStringList fileNames;
    if(dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        this->pathBitstream->setText(fileNames.at(0));
        fileNames.removeAt(0);
    }
    else
        dialog.reject();
}
