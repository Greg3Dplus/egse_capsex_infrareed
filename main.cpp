#include "mainwindow.h"
#include <QApplication>
#include <QtMessageHandler>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        //fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(res);
    if (QDEBUG_PRINT_EN == 0) {  // No qDebug prints on the console
        qInstallMessageHandler(myMessageOutput);
    }
    else { // Normal mode
        qInstallMessageHandler(nullptr);
    }

    QApplication a(argc, argv);
    MainWindow *w;
    bool batch_mode = false;
    bool in_script_list = false;
    QStringList script_list;

    for(quint8 i = 0; i < argc; i++) {

        if (QString::fromUtf8(argv[i]) == "-batch") {
            qDebug() << "Run in batch mode";
            batch_mode = true;
            in_script_list = false;
        }
        else if (QString::fromUtf8(argv[i]) == "-script_list") {
            qDebug() << "Run a list of scripts";
            in_script_list = true;
        }
        else if (in_script_list) {
            qDebug() << "Add in list: " << argv[i];
            script_list.append(argv[i]);
        }
    }

    w = new MainWindow(nullptr, batch_mode);  
    if (!batch_mode)
    {        
        w->show();
        return a.exec();
    }
    else {

        for (quint8 idx = 0; idx < script_list.size(); idx++) {
            qDebug().noquote() << "********************************";
            qDebug().noquote() << "Running script " + QString::number(idx+1,10) + "/" + QString::number(script_list.size(),10) + " ...";
            w->run_script(script_list.at(idx), true);
        }
        return 0;
    }
}
