#ifndef SAVEBMPTHREAD_H
#define SAVEBMPTHREAD_H

#include <QThread>
#include <QPixmap>
#include <QFile>
#include <QDebug>

class saveBMPThread : public QThread
{
    Q_OBJECT

public:
    explicit saveBMPThread();
    void run (void);

    bool add_pixmap(QPixmap pixmap);
    bool add_pixmap(QPixmap pixmap, QString Name, bool script);
    void set_stop(bool st);
    void reset(quint32 num_sequence, double CMV_in_freq, quint16 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 nb_image);
private:
    quint8 nb_read;
    quint8 nb_write;
    bool full_empty;
    QPixmap pixmap_tab[256];
    QString name_tab[256];
    bool stop;
    quint32 num_image_txt;
    quint32 num_sequence;
    double CMV_in_freq;
    quint16 ADC_gain;
    quint8 vramp1;
    quint8 vramp2;
    quint16 offset;
    quint8 nb_image;

signals:
    void NEW_SAVE(QString file_name);
    void END_THREAD();

};

#endif // SAVEBMPTHREAD_H
