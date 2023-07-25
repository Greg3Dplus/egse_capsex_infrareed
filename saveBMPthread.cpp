#include "saveBMPThread.h"

saveBMPThread::saveBMPThread()
{

}

void saveBMPThread::reset(quint32 num_sequence, double CMV_in_freq, quint16 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 nb_image)
{
    this->nb_image = nb_image;

    this->nb_read = 0;
    this->nb_write = 0;
    this->full_empty = false;

    if (nb_image != 0)
        this->stop = false;
    else
        this->stop = true;

    this->num_image_txt = 0;
    this->num_sequence = num_sequence;
    this->CMV_in_freq = CMV_in_freq;
    this->ADC_gain = ADC_gain;
    this->vramp1 = vramp1;
    this->vramp2 = vramp2;
    this->offset = offset;
}

void saveBMPThread::run (void)
{
    while (!this->stop)
    {
        if (this->nb_read != this->nb_write || this->full_empty)
        {
            emit NEW_SAVE("BMP " + this->name_tab[this->nb_read] + " saved");

            QFile file(this->name_tab[this->nb_read]);
            file.open(QIODevice::ReadWrite | QIODevice::Truncate);
            this->pixmap_tab[this->nb_read].save(&file, "BMP");

            this->nb_read++;
            this->full_empty = false;
            if (this->nb_read >= this->nb_image)
                this->stop = true;

        }
    }
    emit END_THREAD();
}

bool saveBMPThread::add_pixmap(QPixmap pixmap)
{
    return add_pixmap(pixmap,"bmp_picture_", false);
}


bool saveBMPThread::add_pixmap(QPixmap pixmap,QString Name, bool script)
{
    if (!this->full_empty)
    {
        if(!script)
            this->name_tab[this->nb_write] = Name + QString::number(this->num_sequence) + "_" + QString::number(this->num_image_txt) + ".bmp";
        else
            this->name_tab[this->nb_write] = Name  + "_" + QString::number(this->num_image_txt) + ".bmp";


        this->num_image_txt++;
        this->pixmap_tab[this->nb_write] = pixmap;
        this->nb_write++;
        if (this->nb_read == this->nb_write)
            this->full_empty = true;
        return true;
    }
    else
        return false;
}


void saveBMPThread::set_stop(bool st)
{
    this->stop = st;
}

