#include "saveimagethread.h"

saveImageThread::saveImageThread()
{

}

void saveImageThread::reset(quint32 num_sequence, double SENS_in_freq, quint16 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 nb_image, quint8 fot_length, quint8 cam_fw_type, double H_Period)
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
    this->SENS_in_freq = SENS_in_freq;
    this->ADC_gain = ADC_gain;
    this->vramp1 = vramp1;
    this->vramp2 = vramp2;
    this->offset = offset;
    this->fot_length = fot_length;
    this->cam_fw_type = cam_fw_type;
    this->H_Period    = H_Period;
}

void saveImageThread::run (void)
{
    while (!this->stop)
    {
        if (this->nb_read != this->nb_write || this->full_empty)
        {
            emit NEW_SAVE("TXT " + this->name_tab[this->nb_read] + " saved");

                this->CU_FRAME_tab[this->nb_read]->create_txt_file(this->name_tab[this->nb_read], this->SENS_in_freq, this->ADC_gain, this->vramp1, this->vramp2, this->offset, this->fot_length, this->cam_fw_type, this->H_Period);

            this->nb_read++;
            this->full_empty = false;
            if (this->nb_read >= this->nb_image)
                this->stop = true;
        }
    }
    emit END_THREAD();
}

bool saveImageThread::add_cu_frame(CU_Frame *CU_frame)
{

        return add_cu_frame(CU_frame,"txt_picture_", false);

}


bool saveImageThread::add_cu_frame(CU_Frame *CU_frame, QString Name, bool script)
{
    if (!this->full_empty)
    {
        if (!script)
            this->name_tab[this->nb_write] = Name + QString::number(this->num_sequence) + "_" + QString::number(this->num_image_txt) + ".txt";
        else
            this->name_tab[this->nb_write] = Name + "_" + QString::number(this->num_image_txt) + ".txt";


        this->num_image_txt++;
        this->CU_FRAME_tab[this->nb_write] = new CU_Frame(CU_frame);
        this->nb_write++;
        if (this->nb_read == this->nb_write)
            this->full_empty = true;
        return true;
    }
    else
        return false;
}



void saveImageThread::set_stop(bool st)
{
    this->stop = st;
}

