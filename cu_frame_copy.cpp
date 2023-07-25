#include "cu_frame.h"

CU_Frame::CU_Frame(quint8 nb_bytes_per_pixel, quint8 cam_fw_type)
{
    this->init_CU_Frame(nb_bytes_per_pixel, cam_fw_type);
}

CU_Frame::CU_Frame(CU_Frame* input_frame) {
    this->copyCU_Frame(input_frame);
}

CU_Frame::CU_Frame(QByteArray header_ar, quint8 nb_bytes_per_pixel, quint8 cam_fw_type, quint8 nb_bits)
{
    this->init_CU_Frame(nb_bytes_per_pixel, cam_fw_type);

    this->setNbBits(nb_bits);
    this->init_pkt(header_ar);
}

CU_Frame::CU_Frame(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file
                   quint8 cam_fw_type,
                   quint8 nb_bits_final,    // After binning

                   quint16 pix_start        ,
                   quint16 pix_last         ,
                   quint16 row_start        ,
                   quint16 nb_row           ,

                   quint16 black_lim ,
                   quint16 white_lim    ,

                   quint8 frame_idx  ,
                   quint8 nb_bytes    ,
                   QByteArray frame_tag  ,
                   quint8 footer    ,
                   quint8 binning_X,
                   quint8 binning_Y,
                   quint8 binmode,   // 0: Sum  1: Average
                   quint8 nb_bits_sensor,    // Shall be 10 or 12
                   quint8 test_mode   ,
                   quint32 integration_time,
                   quint8 ADC_gain,
                   quint16 temperature,
                   quint8 EOS) {

    this->init_CU_Frame(nb_bytes, cam_fw_type);
    this->setNbBits(nb_bits_final);

    QPixArray video_16b = header_footer(
                picture_data,  // Array of pixels. If defined, we use these values instead of input file

                pix_start,
                pix_last,
                row_start,
                nb_row,

                black_lim,
                white_lim,

                nb_bytes,
                frame_tag,
                footer,
                binning_X,
                binning_Y,
                binmode,    // 0: Sum  1: Average
                nb_bits_sensor,    // Shall be 10 or 12
                test_mode,
                integration_time,
                ADC_gain,
                frame_idx,
                temperature,
                EOS);

    this->packet.clear();

    for (quint32 pix_idx = 0; pix_idx < video_16b.size(); pix_idx++) {
        quint32 pix = video_16b.at(pix_idx);
        for (quint8 byte_num = 0; byte_num < this->nb_bytes_per_pixel; byte_num++) {
            this->packet.append(static_cast<qint8>((pix >> 8*(this->nb_bytes_per_pixel-byte_num-1)) & 0xff));
        }
    }
}

void CU_Frame::set_ref_height(quint16 ref_height) {
    this->ref_height = ref_height;
}

void CU_Frame::init_CU_Frame(quint8 nb_bytes_per_pixel, quint8 cam_fw_type)
{
    this->time = new QTime();
    this->time->start();
    this->nb_bytes_per_pixel = nb_bytes_per_pixel;
    this->setCamType(cam_fw_type);

    if (this->cam_fw_type == CAM_FW_CO2M){
        this->setCMVHeaderSize(116);
    }
    else {
        this->setCMVHeaderSize(29);
    }
}

void CU_Frame::setNbBits(quint8 nb_bits) {
    this->nb_bits = nb_bits;
}

void CU_Frame::setNbBytes_per_pix(quint8 nb_bytes_per_pixel) {
    this->nb_bytes_per_pixel = nb_bytes_per_pixel;
}

void CU_Frame::setCMVHeaderSize(quint8 CMV_HEADER_SIZE) {
    this->CMV_HEADER_SIZE = CMV_HEADER_SIZE;
    //qDebug().noquote()  << "set CMV_HEADER_SIZE to " + QString::number(CMV_HEADER_SIZE);
}

quint8 CU_Frame::getCMVHeaderSize(void) {
    return this->CMV_HEADER_SIZE;
}

void CU_Frame::setCamType(quint8 cam_fw_type) {
    this->cam_fw_type = cam_fw_type;
}

bool CU_Frame::init_pkt(QByteArray header_ar) {
    this->packet.clear();
    this->packet.append(header_ar);

    if (this->getPacketSize() < this->nb_bytes_per_pixel*this->CMV_HEADER_SIZE) {
        emit this->CU_error();
        return false;
    }

    //qDebug().noquote()  << "packet len = 0x" + QString::number(this->getPacketLength(),16);
    //qDebug().noquote()  << "footer size = 0x" + QString::number(this->getFooter_size(),16);
    //print_data_Bytes(this->packet,4*12);

    // Check that packet size is correct
    if (this->getPacketSize() != this->compute_ref_PktSize(this->getWidth(), this->getHeight(), this->getFooter_size())) {
        qDebug () << "ERROR: Packet size = " + QString::number(this->getPacketSize(), 10) + ", expected " + QString::number(this->compute_ref_PktSize(this->getWidth(), this->getHeight(), this->getFooter_size()), 10);
        emit this->CU_error();
        return false;
    }

    // Check that packet length field of the header is correct
    if (this->getPacketLength() != this->compute_ref_PktLength()){
        qDebug () << "ERROR: Packet length (header) = " + QString::number(this->getPacketLength(), 10) + ", expected " + QString::number(this->compute_ref_PktLength(), 10);
        return false;
    }

    return true;
}

quint8 CU_Frame::getByte(quint32 address) {
    return static_cast<quint8>(this->packet.at(static_cast<int>(address)));
}

quint32 CU_Frame::getData(quint32 address, quint8 nb_bits) {
    return this->getData(address, nb_bits, false);
}

quint32 CU_Frame::getData(quint32 address, quint8 nb_bits, bool no_shift) {
    if(this->cam_fw_type == CAM_FW_CO2M){
        quint32 data = 0;
        for (quint8 byte_num = 0; byte_num < this->nb_bytes_per_pixel; byte_num++) {
            quint8 Byte_val = this->getByte(this->nb_bytes_per_pixel*address+byte_num);
            data |= static_cast<quint32>(Byte_val << (8 * byte_num));
        }
        return data >> (8*this->nb_bytes_per_pixel-nb_bits);
    }
    else {
        quint32 data = 0;
        for (quint8 byte_num = 0; byte_num < this->nb_bytes_per_pixel; byte_num++) {
            quint8 Byte_val = this->getByte(this->nb_bytes_per_pixel*address+byte_num);
            data |= static_cast<quint32>(Byte_val << (8*(this->nb_bytes_per_pixel-byte_num-1)));
        }
        if (no_shift) {
            return data;
        }
        return data >> (8*this->nb_bytes_per_pixel-nb_bits);
    }

}

// num_field input is given in "pixels", not in Bytes
quint8 CU_Frame::getHeader_field(quint16 num_field) {
    if (this->cam_fw_type == CAM_FW_CO2M){
        return static_cast<quint8>(this->getData(4*num_field+3, 8));
    }
    else {
        return static_cast<quint8>(this->getData(num_field, 8));
    }
}

// num_field input is given in "pixels", not in Bytes
quint8 CU_Frame::getFooter_field(quint16 num_field) {
    if (this->cam_fw_type == CAM_FW_CO2M){
        return static_cast<quint8>(this->getData(this->CMV_HEADER_SIZE + this->getWidth() * this->getHeight() + 4*num_field+3, 8));
    }
    else {
        return static_cast<quint8>(this->getData(this->CMV_HEADER_SIZE + this->getWidth() * this->getHeight() + num_field, 8));
    }
}

quint32 CU_Frame::getPixel(quint16 num_line, quint16 num_pix, quint8 nb_bits) {

    quint32 num_pixel = this->getWidth() * num_line + num_pix;
    if ((this->nb_bytes_per_pixel * (this->CMV_HEADER_SIZE + num_pixel) + this->nb_bytes_per_pixel-1) > this->getPacketSize()-1) {
        qDebug().noquote()  << "Unexpected packet size = " << this->getPacketSize();
    }
    else {
        if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
            return this->getData(this->CMV_HEADER_SIZE + num_pixel, nb_bits, true);
        }
        else {
            return this->getData(this->CMV_HEADER_SIZE + num_pixel, nb_bits);
        }
    }
    return 0;
}

quint32 CU_Frame::getPixelRGB(quint16 num_line, quint16 num_pix, quint8 nb_bits) {

    quint32 num_pixel = this->getWidth()*4 * num_line + num_pix;

    quint32 pix = static_cast<quint32>(((this->getByte(this->nb_bytes_per_pixel*this->CMV_HEADER_SIZE + this->nb_bytes_per_pixel* num_pixel)) << 8) |
                                       (this->getByte(this->nb_bytes_per_pixel*this->CMV_HEADER_SIZE + this->nb_bytes_per_pixel* num_pixel + 1)));

    return pix >> (16 - nb_bits);

}

quint16 CU_Frame::getWidth(void) {
    return  static_cast<quint16> ((this->getHeader_field(21) << 8) | this->getHeader_field(20)) +1;
}

quint16 CU_Frame::getHeight(void) {
    if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
        return this->ref_height;
    }
    else {
        return this->getPktHeight();
    }
}

quint16 CU_Frame::getPktHeight(void) {
    return static_cast<quint16> ((this->getHeader_field(23) << 8) | this->getHeader_field(22)) +1;
}

quint16 CU_Frame::getOffset_x(void) {
    return  static_cast<quint16> ((this->getHeader_field(25) << 8) | this->getHeader_field(24));
}

quint16 CU_Frame::getOffset_y(void) {
    return  static_cast<quint16> ((this->getHeader_field(27) << 8) | this->getHeader_field(26));
}

quint16 CU_Frame::getTemp_reg(void) {
    return static_cast<quint16> ((this->getHeader_field(18) << 8) | this->getHeader_field(17));
}

double CU_Frame::getTemp_val(double cmv_in_freq) {
    // Compute temperature according to datasheet
    double offset      = 1000.0 * cmv_in_freq /40.0;
    double temperature = (this->getTemp_reg() - offset) * 0.3 * 40.0 / cmv_in_freq;

    return temperature;
}

quint8 CU_Frame::getFrame_ID(void) {
    return this->getHeader_field(5);
}

quint8 CU_Frame::getFooter_size(void) {
    return this->getHeader_field(28);
}

QTime* CU_Frame::getTime(void) {
    return this->time;
}

quint8 CU_Frame::getNb_bits(void) {
    return this->nb_bits;

    /*quint8 pix_res = (this->getHeader_field(4) >> 5) & 0x3;

    if      (pix_res == 0) {
        return 12;
    }
    else if (pix_res == 2) {
        return 10;
    }
    else {
        return 8;
    }*/
}

quint32 CU_Frame::compute_ref_PktSize(quint16 width, quint16 height, quint8 footer_size) {
    return compute_ref_PktSize(width, height, this->nb_bytes_per_pixel, this->CMV_HEADER_SIZE, footer_size);
}

// All inputs are given in "pixels"
// The return value is given in Bytes
quint32 CU_Frame::compute_ref_PktSize(quint16 width, quint16 height, quint8 nb_bytes, quint8 CMV_HEADER_SIZE, quint8 footer_size) {
    return nb_bytes*CMV_HEADER_SIZE + nb_bytes * width * height + nb_bytes * footer_size;
}

QString CU_Frame::getFrame_tag(void) {
    QString frame_tag = "0x";
    for (quint8 i=13; i >= 6; i--)
    {
        frame_tag.append(QString::number(this->getHeader_field(i), 16).rightJustified(2, '0'));
    }

    return frame_tag;
}

QByteArray CU_Frame::getPacket(void) {
    return this->packet;
}

quint32 CU_Frame::getPacketSize(void) {
    return static_cast<quint32>(this->packet.size());
}

quint32 CU_Frame::getPacketLength(void) {

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 4; data_num++) {
        data |= static_cast<quint32> ((this->getHeader_field(data_num)) << (8*data_num));
    }

    return data;
}

// Reference of the PktLen field inside the Header
quint32 CU_Frame::compute_ref_PktLength(void) {
    if (this->cam_fw_type == CAM_FW_CO2M) {
        return this->CMV_HEADER_SIZE + this->getWidth() * this->getPktHeight() + this->getFooter_size();
    }
    else {
        return this->nb_bytes_per_pixel* (this->CMV_HEADER_SIZE + this->getWidth() * this->getPktHeight() + this->getFooter_size());
    }
}

quint32 CU_Frame::getExp_reg(void) {
    quint8 data_start_ad = 14;

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 3; data_num++) {
        data |= static_cast<quint32> ((this->getHeader_field(data_start_ad + data_num)) << (8*data_num));
    }

    return data;
}

quint32 CU_Frame::getPixSum(void) {
    quint8 data_start_ad = 0;

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 4; data_num++) {
        data |= static_cast<quint32> ((this->getFooter_field(data_start_ad + data_num)) << (8*data_num));
    }

    return data;
}

quint32 CU_Frame::getBlackCount(void) {
    quint8 data_start_ad = 4;  // In number of "pixels", not Bytes

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 3; data_num++) {
        data |= static_cast<quint32> ((this->getFooter_field(data_start_ad + data_num)) << (8*data_num));
    }

    return data;
}

quint32 CU_Frame::getWhiteCount(void) {
    quint8 data_start_ad = 7;  // In number of "pixels", not Bytes

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 3; data_num++) {
        data |= static_cast<quint32> ((this->getFooter_field(data_start_ad + data_num)) << (8*data_num));
    }

    return data;
}

quint32 CU_Frame::getCRC(void) {
    quint8 data_start_ad = 10;  // In number of "pixels", not Bytes

    quint32 data = 0;
    for (quint8 data_num = 0; data_num < 4; data_num++) {
        data |= static_cast<quint32> ((this->getFooter_field(data_start_ad + data_num)) << (8*data_num));
    }

    return data;
}

bool CU_Frame::isEOS(void) {
    return (this->getHeader_field(4) & 0x80) != 0;
}

void CU_Frame::create_txt_file(QString file_name,  // "image_test.txt",
                               double cmv_in_freq, quint8 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 fot_length, quint8 cam_fw_type)
{

    QFile outputFile(file_name);
    QFileInfo fileinfo(file_name);

    QDir dir = QDir();

    if (! dir.exists(fileinfo.path())) {
        dir.mkpath(fileinfo.path());
    }

    // Create new file
    if (outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream out(&outputFile);

        out << "Saved Path       : " << fileinfo.absoluteFilePath() << "\n";

        // qDebug().noquote()  << "Print header";
        // print_data_Bytes(this->packet, 3*this->CMV_HEADER_SIZE);

        //qDebug().noquote()  << "cam_type txt = " + QString::number(cam_type);
        //qDebug().noquote()  << "-------";

        int EOS               = (this->getHeader_field(4) & 0x80) >> 7;
        quint8 nb_bits        = this->getNb_bits();
        quint8 binning_x;
        quint8 binning_y;
        quint8 frame_type;
        if (cam_fw_type == CAM_FW_3DIPCC0746) {
            binning_x         = (this->getHeader_field(4) & 0x18) >> 3;
            binning_y         = binning_x;
            frame_type        = (this->getHeader_field(4) & 0x07) >> 0;
        }
        else if (cam_fw_type == CAM_FW_3DIPCC0802) {
            binning_x         = (this->getHeader_field(4) & 0x30) >> 4;
            binning_y         = (this->getHeader_field(4) & 0x07) >> 0;
            frame_type        = (this->getHeader_field(4) & 0x08) >> 3;
        }
        else {
            binning_x         = (this->getHeader_field(4) & 0x30) >> 4;
            binning_y         = (this->getHeader_field(4) & 0x0F) >> 0;
            frame_type        = (this->getHeader_field(4) & 0x40) >> 6;

            binning_x = static_cast<quint8>(pow(2, binning_x));
            binning_y++;

            //qDebug().noquote()  << "cam_type = " + QString::number(this->cam_type);
            //qDebug().noquote()  << "CMV_HEADER = " + QString::number(this->CMV_HEADER_SIZE);
            //qDebug().noquote()  << "nb_bits = " + QString::number(this->nb_bits);
            //qDebug().noquote()  << "nb_bytes_perpixel = " + QString::number(this->nb_bytes_per_pixel);
            //qDebug().noquote()  << "binning_x = " + QString::number(binning_x);
            //qDebug().noquote()  << "binning_y = " + QString::number(binning_y);
        }
        quint32 integration_time  = getExp_reg();
        quint8 footer_size       = this->getHeader_field(28);

        // Compute temperature according to datasheet
        double temperature = this->getTemp_val(cmv_in_freq);

        out << "End Of Sequence  : " << EOS << "\n";

        out << "Mode             : " << nb_bits << " bits per pixel\n";

        if (cam_fw_type == CAM_FW_3DIPCC0746 && binning_x > 0) {
            out << "Binning          : 2x2 \n";
        }
        else {
            out << "Binning X        : " << binning_x << "\n";
            out << "Binning Y        : " << binning_y << "\n";
        }

        if (frame_type == 0) {
            out << "Frame type       : Normal mode \n";
        }
        else {
            out << "Frame type       : Test mode \n";
        }

        out << "Frame ID         : "   << this->getFrame_ID() << "\n";

        out << "Frame tag        : " << this->getFrame_tag() << "\n";

        double exp_time_int = (integration_time + 0.43*fot_length) * 129/cmv_in_freq;
        out << "Integration time : "   << dec << exp_time_int  << " us" << " (register set to 0x" << hex << integration_time << ")\n";


        // out << "Temperature      : 0x" << hex << temperature << "\n";

        out << "Temperature      : " << temperature << " °C\n";
        out << "Window width     : "   << dec << this->getWidth() << "\n";
        out << "Window height    : "   << dec << this->getHeight() << "\n";
        out << "Offset X         : "   << dec << this->getOffset_x() << "\n";
        out << "Offset Y         : "   << dec << this->getOffset_y() << "\n";
        out << "Footer size      : "   << dec << footer_size << " bytes \n";
        out << "ADC Gain         : "   << dec << ADC_gain << "\n";
        out << "Vramp1           : "   << dec << vramp1 << "\n";
        out << "Vramp2           : "   << dec << vramp2 << "\n";
        out << "Offset           : "   << dec << offset << "\n";

        out <<  "\nPixel Values :\n";


        // Print pixel values
        for(quint16 line = 0; line <= this->getHeight()-1; line++) {
            for(quint16 pix = 0; pix < this->getWidth(); pix++) {

                quint32 pixel = getPixel(line, pix, this->getNb_bits());

                out << " " << dec <<QString("%1").arg(pixel, 4, 10, QChar(' ')); // Equivalent printf(%5d)

                if (pix == this->getWidth() - 1) {
                    out << endl;
                }
            }
        }

        out << endl;

        // Print footer
        if (footer_size != 0) {

            // Print footer into file
            // for (int i=0; i <= 13; i++) {
            // out << "Footer["<< i <<"] ="<< footer_vec[i] << "\n";
            // }
            quint32 Pix_sum   = this->getPixSum();
            quint32 Nb_dark   = this->getBlackCount();
            quint32 Nb_white  = this->getWhiteCount();
            quint32 CRC       = this->getCRC();

            out << "Sum of pixels    : " << Pix_sum << "\n";
            out << "Black pixels     : " << Nb_dark << "\n";
            out << "White pixels     : " << Nb_white << "\n";
            out << "CRC              : 0x" << hex << CRC << "\n";

        }

        outputFile.close();
    }
    else {
        qDebug () << "ERROR: File " << file_name << " not opened";
    }
}

/*QByteArray CU_Frame::map_video_pkt(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file
                                   QString in_file     ,  // Name of the file
                                   QString out_file    ,  // Name of the file

                                   quint16 pix_start   ,
                                   quint16 pix_last    ,
                                   quint16 row_start   ,
                                   quint16 nb_row,

                                   quint16 black_lim,
                                   quint16 white_lim,

                                   quint8 nb_bytes,
                                   QByteArray frame_tag,
                                   quint8 footer,
                                   quint8 binning,
                                   quint8 nb_bits,    // Shall be 10 or 12
                                   quint8 test_mode,
                                   quint8 nb_image,
                                   quint8 num_1st_image,
                                   quint8 num_last_image,
                                   quint32 integration_time,
                                   quint8 ADC_gain) {

    qDebug () << "nb_image = %d\n" + QString::number(nb_image, 10);

    QByteArray video_Byte;
    video_Byte.clear();

    quint8 last_image = num_last_image;

    for (quint8 frame_idx = num_1st_image ; frame_idx <= last_image; frame_idx++) {

        //quint8 nb_max = nb_image-1;
        qDebug () << "Writing frame frame_idx/nb_max ...";

        quint8 EOS = (frame_idx == nb_image-1 ? 0x1: 0x0);

        video_Byte.append(map_frame_pkt(
                              picture_data,  // Array of pixels. If defined, we use these values instead of input file
                              in_file,       // Name of the file
                              out_file,      // Name of the file

                              pix_start,
                              pix_last,
                              row_start,
                              nb_row,

                              black_lim,
                              white_lim,

                              frame_idx,         // frame_idx
                              nb_bytes,
                              frame_tag,
                              footer,
                              binning,
                              nb_bits,    // Shall be 10 or 12
                              test_mode,
                              integration_time,
                              ADC_gain,
                              EOS,
                              PROTOCOL_ID));

    }

    return video_Byte;
}*/

QPixArray CU_Frame::header_footer(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file
                                  quint16 pix_start   ,
                                  quint16 pix_last    ,
                                  quint16 row_start   ,
                                  quint16 nb_row      ,

                                  quint16 black_lim ,
                                  quint16 white_lim ,

                                  quint8 nb_bytes    ,
                                  QByteArray frame_tag   ,    // Array of 8 Bytes
                                  quint8 footer      ,   // 0 or 1
                                  quint8 binning_X,
                                  quint8 binning_Y,
                                  quint8 binmode,
                                  quint8 nb_bits_sensor,    // Shall be 10 or 12
                                  quint8 test_mode       ,
                                  quint32 integration_time,
                                  quint8 ADC_gain        ,
                                  quint8 frame_idx       ,
                                  quint16 temperature,
                                  quint8 EOS_flag) {


    QPixArray frame_16b = map_header(
                frame_idx,
                EOS_flag,
                nb_bits_sensor,
                nb_bytes,
                test_mode,
                pix_start,
                pix_last,
                footer,
                binning_X,
                binning_Y,
                frame_tag,
                row_start,
                nb_row,
                integration_time,
                temperature,
                ADC_gain);

    //qDebug () << "After map_header:";
    //frame_16b.print_data();

    QPixArray picture_data_int;

    if (picture_data.size() > 0) {
        picture_data_int.append(picture_data);
    }
    else {

        /*my $in_file      = $args->{in_file};
    my $in_file_name = remove_extension($in_file);

    open(IN_FILE, '<', $in_file) or die sprintf("ERROR in header_footer function when opening file %s\n", $in_file);

    while (read(IN_FILE, my $buff, 2)){ # Read chunks of 2 characters (16 bits)

      my @pixel = unpack("C*", $buff);
      my $pix_value = ($pixel[0] << 8) | $pixel[1];

      push @picture_data, $pix_value;
    }

    close(IN_FILE);*/
    }

    quint16 size_row = pix_last - pix_start + 1;
    quint16 nb_rows  = nb_row;

    // Windowing
    QPixArray picture_data_window;
    picture_data_window.clear();
    for(quint16 num_row = row_start; num_row < row_start+nb_rows; num_row++) {
        for(quint16 num_pix = pix_start; num_pix <= pix_last; num_pix++) {
            picture_data_window.append(picture_data_int.at(2048*num_row + num_pix));
        }
    }

    //qDebug () << "Print picture_data_int before binning:";
    //picture_data_int.print_data();

    // Binning in X
    QPixArray picture_data_bin_1;
    picture_data_bin_1.clear();

    for(quint16 num_row = 0; num_row < nb_rows; num_row++) {
        for(quint16 num_pix = 0; num_pix < size_row/binning_X; num_pix++) {
            QPixArray bin_ar;
            bin_ar.clear();
            for (quint16 num_subpix = 0; num_subpix < binning_X; num_subpix++) {
                bin_ar.append(picture_data_window.at(num_row*size_row + binning_X*num_pix+num_subpix));
            }
            picture_data_bin_1.append(this->binning(bin_ar, 0, 0));   // 0: Sum  1: Average
        }
    }
    size_row /= binning_X;

    // Binning in Y
    QPixArray picture_data_bin_2;
    picture_data_bin_2.clear();

    for(quint16 num_row = 0; num_row < nb_rows/binning_Y; num_row++) {

        for(quint16 num_pix = 0; num_pix < size_row; num_pix++) {
            QPixArray bin_ar;
            bin_ar.clear();
            for (quint16 num_subrow = 0; num_subrow < binning_Y; num_subrow++) {
                bin_ar.append(picture_data_bin_1.at((binning_Y*num_row+num_subrow)*size_row + num_pix));
            }

            picture_data_bin_2.append(this->binning(bin_ar, 0, 0));   // 0: Sum  1: Average

        }
    }
    nb_rows  /= binning_Y;

    QPixArray picture_data_bin_3;
    picture_data_bin_3.clear();

    if (binmode) {
        // We shall always divide by the next power of 2
        quint32 divider = static_cast<quint32>(pow(2, log2_int(binning_X * binning_Y)));

        for (quint32 pix_idx = 0; pix_idx < picture_data_bin_2.size(); pix_idx++) {
            picture_data_bin_3.append(picture_data_bin_2.at(pix_idx) / divider);
        }
    }
    else {
        picture_data_bin_3.append(picture_data_bin_2);
    }

    //qDebug () << "Print picture_data_int after binning:";
    //picture_data_bin.print_data();

    // Compute pix_sum, nb_dark and nb_white fields

    quint32 pix_sum  = 0;
    quint32 nb_dark  = 0;
    quint32 nb_white = 0;

    for (quint32 pix_idx = 0; pix_idx < static_cast<quint32>(size_row*nb_rows); pix_idx++) {
        quint32 pix_value = picture_data_bin_3.at(pix_idx);
        pix_sum += pix_value;

        if (pix_value <= black_lim){
            nb_dark++;
        }
        if (pix_value >= white_lim){
            nb_white++;
        }
    }

    //qDebug () << "nb_white = " + QString::number(nb_white, 10);
    //qDebug () << "nb_dark = " + QString::number(nb_dark, 10);

    // Add all the EOL flags
    for (quint16 num_row = 0; num_row < nb_rows; num_row++) {
        quint8 EOF_flag = 0;
        if (footer == 0 && num_row == nb_rows-1) {
            EOF_flag = 1;
        }

        quint8 EOS_data = 0;
        if (footer == 0 && num_row == nb_rows-1 && EOS_flag == 1) {
            EOS_data = 1;
        }

        // Map the data
        QPixArray pix_line;
        pix_line.clear();

        for (quint32 num_pix = num_row * size_row; num_pix < static_cast<quint32>((num_row+1) * size_row); num_pix++) {
            pix_line.append(picture_data_bin_3.at(num_pix));
        }

        frame_16b.append(map_data(pix_line,
                                  0,
                                  1,
                                  EOF_flag,
                                  EOS_data, 0,
                                  this->nb_bits));

        //qDebug () << "Print frame_16b after map_data:";
        //frame_16b.print_data();
    }

    if (footer) {
        // Generate footer (without CRC)
        frame_16b.append(map_footer(pix_sum ,
                                    nb_dark,
                                    nb_white   ));

        frame_16b.append(map_crc(frame_16b,
                                 EOS_flag));
    }

    /*if (defined out_file) {
        my $out_file     = $args->{out_file};

        open(OUT_FILE, '>>', $out_file) or die sprintf("ERROR in header_footer function when opening file %s\n", $out_file);
        binmode(OUT_FILE);

        // Print all the frame into output file
        foreach my $data (@frame_16b) {
            print OUT_FILE pack("H*", $data);
        }

        close(OUT_FILE);
    }*/

    return frame_16b;
}

// Compute and map the CRC
QPixArray CU_Frame::map_crc(QPixArray data_16b, quint8 EOS_flag) {

    quint32 crc_out = 0;

    for (quint32 data_idx = 0; data_idx < data_16b.size(); data_idx++) { // Read 1 character (8 bits)
        quint32 data = data_16b.at(data_idx);

        crc_out = calc_crc(
                    static_cast<quint8>(data >> 8),       // 8 bits
                    crc_out          // 32 bits
                    );

        crc_out = calc_crc(
                    data & 0xff,     // 8 bits
                    crc_out          // 32 bits
                    );
    }

    QPixArray crc_Byte;
    crc_Byte.clear();

    crc_Byte.append((crc_out >>  0) & 0xff);
    crc_Byte.append((crc_out >>  8) & 0xff);
    crc_Byte.append((crc_out >> 16) & 0xff);
    crc_Byte.append((crc_out >> 24) & 0xff);

    return map_data(
                crc_Byte,
                0,        // SOF
                0,        // EOL
                1,        // EOF
                EOS_flag,
                1,        // h_f
                8         // nb_bits
                );
}

// CRC engine
quint32 CU_Frame::calc_crc(quint8 data_in, quint32 fcs) {

    QByteArray crc;         // 1 element per bit
    crc.clear();
    for (quint8 bit = 0; bit < 32; bit++) {
        crc.append(static_cast<quint8>(0));
    }

    quint8 crc_feedback;

    for (quint8 bit = 0; bit < 32; bit++) {
        crc[bit] = (~ ((fcs >> bit) & 0x1)) & 0x1;
    }

    QByteArray data;
    data.clear();
    for (quint8 bit = 0; bit < 8; bit++) {
        data.append(static_cast<quint8>(0));
    }

    for (quint8 bit = 0; bit < 8; bit++) {
        data[bit] = ((data_in >> bit) & 0x1) & 0x1;
    }

    for (quint8 I = 0; I < 8; I++) {
        crc_feedback      = static_cast<quint8>(crc.at(0)) ^ static_cast<quint8>(data.at(I));

        QByteArray crc_2;         // 1 element per bit
        crc_2.clear();

        for (quint8 idx = 0; idx < 5; idx++) {
            crc_2.append(crc.at(idx+1));
        }
        crc_2.append(crc.at(6)  ^ crc_feedback);
        crc_2.append(crc.at(7));
        crc_2.append(crc.at(8));
        crc_2.append(crc.at(9)  ^ crc_feedback);
        crc_2.append(crc.at(10) ^ crc_feedback);

        for (quint8 idx = 10; idx < 15; idx++) {
            crc_2.append(crc.at(idx+1));
        }

        crc_2.append(crc[16] ^ crc_feedback);

        for (quint8 idx = 16; idx < 19; idx++) {
            crc_2.append(crc.at(idx+1));
        }

        crc_2.append(crc.at(20) ^ crc_feedback);
        crc_2.append(crc.at(21) ^ crc_feedback);
        crc_2.append(crc.at(22) ^ crc_feedback);
        crc_2.append(crc.at(23));
        crc_2.append(crc.at(24) ^ crc_feedback);
        crc_2.append(crc.at(25) ^ crc_feedback);
        crc_2.append(crc.at(26));
        crc_2.append(crc.at(27) ^ crc_feedback);
        crc_2.append(crc.at(28) ^ crc_feedback);
        crc_2.append(crc.at(29));
        crc_2.append(crc.at(30) ^ crc_feedback);
        crc_2.append(crc.at(31) ^ crc_feedback);
        crc_2.append(static_cast<qint8>(crc_feedback));

        crc = (crc_2);
    }

    quint32 crc_val = 0;
    for (quint8 bit = 0; bit < 32; bit++) {
        crc_val |= static_cast<quint32>(static_cast<quint8>(crc.at(bit)) << bit);
    }

    // return the CRC result
    return (~crc_val) & 0xffffffff;
}

// Output is an array of 16-bit words
QPixArray CU_Frame::map_header(quint8 frame_idx       ,
                               quint8 EOS_flag,
                               quint8 nb_bits_sensor,
                               quint8 nb_bytes        ,
                               quint8 test_mode       ,
                               quint16 pix_start       ,
                               quint16 pix_last        ,
                               quint8 footer          ,
                               quint8 binning_X,
                               quint8 binning_Y,
                               QByteArray frame_tag       ,     // Array of 8 Bytes
                               quint16 row_start       ,
                               quint16 nb_row          ,  // Number of lines
                               quint32 integration_time,
                               quint16 temperature,
                               quint8 ADC_gain) {

    quint8 pix_res          = (nb_bits_sensor == 12 ? 0x0: 0x2);
    if (nb_bytes == 1) {
        pix_res         = 0x3;
    }
    if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
        pix_res          = (nb_bits_sensor == 12 ? 0: 1);
    }

    quint8  frame_type       = (test_mode == 0 ? 0x0: 0x7);
    if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
        frame_type = test_mode;
    }

    quint16 window_width     = (pix_last - pix_start) / binning_X;  // Width in nb of pixels -1
    quint16 window_height    = (nb_row-1) / binning_Y;
    if (this->cam_fw_type == CAM_FW_3DIPCC0802 && binning_Y != 1 && binning_Y != 2 && binning_Y != 4 && binning_Y != 8) {
        window_height    = (nb_row-1) / 2;
    }
    quint16 offset_x         = pix_start;
    quint8  footer_size      = (footer == 1 ? 0xE: 0x0);

    quint32 pkt_size         = this->compute_ref_PktSize(window_width+1, window_height+1, footer_size);

    // Generate Header
    QPixArray header;
    header.clear();
    header.append(pkt_size >>  0 & 0xFF);
    header.append(pkt_size >>  8 & 0xFF);
    header.append(pkt_size >> 16 & 0xFF);
    header.append(pkt_size >> 24 & 0xFF);

    if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
        header.append(static_cast<quint16>(EOS_flag     << 7 |
                                           pix_res      << 6 |
                                           static_cast<quint8>(log2_int(binning_X))    << 4 |
                                           frame_type   << 3 |
                                           (binning_Y-1)    << 0));
    }
    else {
        header.append(static_cast<quint16>(EOS_flag     << 7 |
                                           pix_res         << 5 |
                                           binning_X << 3 |
                                           frame_type      << 0 ));  // Pixel is 8 bits wide for header generation
    }

    header.append(frame_idx);
    header.append(frame_tag);
    header.append(integration_time >> 0 & 0xFF);
    header.append(integration_time >> 8 & 0xFF);
    header.append(integration_time >> 16 & 0xFF);
    header.append((temperature >> 0) & 0xFF);
    header.append((temperature >> 8) & 0xFF);
    header.append(ADC_gain);
    header.append(window_width >> 0 & 0xFF);
    header.append(window_width >> 8 & 0xFF);
    header.append(window_height >> 0 & 0xFF);
    header.append(window_height >> 8 & 0xFF);
    header.append(offset_x >> 0 & 0xFF);
    header.append(offset_x >> 8 & 0xFF);
    header.append(row_start >> 0 & 0xFF);
    header.append(row_start >> 8 & 0xFF);
    header.append(footer_size);

    //qDebug () << "integration time = " + QString::number(integration_time, 10);

    return map_data(header, 1, 1, 0, 0, 1, 8);
}

// Mode = 0: Sum
// Mode = 1: Average
quint32 CU_Frame::binning(QPixArray data, quint8 debug, quint8 binmode) {

    if (debug) {
        qDebug () << "Enter binning function:";
    }

    quint32 sum = 0;
    for (quint8 data_idx = 0; data_idx < data.size(); data_idx++) {
        sum += data.at(data_idx);

        //printf("  Data = 0x%02X\n", $data) if $args->{debug};
    }

    if (binmode) {
        // We shall always divide by the next power of 2
        quint32 divider = static_cast<quint32>(pow(2, log2_int(data.size())));
        return sum / divider;
    }
    else {
        return sum;
    }
}

// Input is array of data on 12 bits max
// Output is same data on 16 bits
QPixArray CU_Frame::map_data(QPixArray data, quint8 SOF_flag, quint8 EOL_flag, quint8 EOF_flag, quint8 EOS_flag, quint8 h_f, quint8 nb_bits) {

    //die "Unexpected h_f value\n" unless ($args->{h_f} <= 1);

    QPixArray data_16b;
    data_16b.clear();

    for (quint16 pix_num = 0; pix_num < data.size(); pix_num++) {

        quint32 pix;
        if (nb_bits > 8*this->nb_bytes_per_pixel) {
            // In this case, the most significant bits of the pixel are removed
            pix = data.at(pix_num);
        }
        else {
            pix = static_cast<quint32>(data.at(pix_num) << (8*this->nb_bytes_per_pixel - nb_bits));
            if (this->cam_fw_type == CAM_FW_3DIPCC0802 and !h_f) {
                pix = static_cast<quint32>(data.at(pix_num));
            }
        }

        quint8 TUSER_TLAST = 0;
        if (pix_num == 0) {
            if    (SOF_flag == 1) {
                TUSER_TLAST = 2;
            }
        }

        if (pix_num == data.size()-1) {
            if    (EOS_flag == 1) {
                TUSER_TLAST = 3;
            }
            else if (EOF_flag == 1) {
                TUSER_TLAST = 4;
            }
            else if (EOL_flag == 1) {
                TUSER_TLAST = 1;
            }
        }

        quint8 h_f_int = h_f;

        if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
            TUSER_TLAST = 0;
            h_f_int     = 0;
        }

        quint16 data_out = static_cast<quint16>((pix << 0) |
                                                (h_f_int << 3) |   // 0x01 to indicate this is a header/footer
                                                TUSER_TLAST);
        data_16b.append(data_out);
    }

    //qDebug () << "Print data_16b in map_data:";
    //data_16b.print_data();

    return data_16b;
}

// Map the footer excluding CRC
// Output is an array of 16-bit word
QPixArray CU_Frame::map_footer (quint32 pix_sum, quint32 nb_dark, quint32 nb_white) {

    QPixArray footer;
    footer.clear();

    // Generate footer

    footer.append(pix_sum >> 0 & 0xFF);
    footer.append(pix_sum >> 8 & 0xFF);
    footer.append(pix_sum >> 16 & 0xFF);
    footer.append(pix_sum >> 24 & 0xFF);
    footer.append(nb_dark >> 0 & 0xFF);
    footer.append(nb_dark >> 8 & 0xFF);
    footer.append(nb_dark >> 16 & 0xFF);
    footer.append(nb_white >> 0 & 0xFF);
    footer.append(nb_white >> 8 & 0xFF);
    footer.append(nb_white >> 16 & 0xFF);

    return map_data(footer, 0, 0, 0, 0, 1, 8);
}

void CU_Frame::print_frame(void) {
    this->print_frame("");
}

void CU_Frame::print_frame(QString frame_name) {
    qDebug () << "Print Frame " + frame_name;
    qDebug () << "Size of frame = " + QString::number(this->getPacketSize());

    print_data_Bytes(this->packet);
}


void CU_Frame::copyCU_Frame(CU_Frame* input_frame)
{
    this->nb_bytes_per_pixel = input_frame->nb_bytes_per_pixel;
    this->packet             = input_frame->getPacket();
    this->nb_bits            = input_frame->nb_bits;
    this->cam_fw_type        = input_frame->cam_fw_type;
    this->CMV_HEADER_SIZE    = input_frame->CMV_HEADER_SIZE;
    this->ref_height         = input_frame->ref_height;
}

QPixmap CU_Frame::toPixMap() {
    // Read 2 bytes each time from the array to form a pixel
    static quint8 final_pic [4*MAX_WIDTH*MAX_HEIGHT];

    quint16 width = this->getWidth();

    // B&W
    for(quint16 line = 0; line < this->getHeight(); line++) {
        for(quint16 pix = 0; pix < width; pix++) {

            quint32 pixel =  this->getPixel(line, pix, 8);
            if (this->cam_fw_type == CAM_FW_3DIPCC0802) {
                pixel = pixel >> (this->getNb_bits() - 8);
            }

            final_pic[4*(width*line + pix)]   = static_cast<quint8>(pixel);
            final_pic[4*(width*line + pix)+1] = static_cast<quint8>(pixel);
            final_pic[4*(width*line + pix)+2] = static_cast<quint8>(pixel);
            final_pic[4*(width*line + pix)+3] = 0xff;
        }
    }

    int step = 0;

    // QImage needs an alignment with 32-bits. We cannot use Format GrayScale 8
    return QPixmap::fromImage(QImage(final_pic, this->getWidth(), this->getHeight(), step, QImage::Format::Format_RGB32));
}

bool CU_Frame::compare(CU_Frame* ref_frame, QVector<quint32> ignore_byte_idx) {

    if (this->getPacketSize() != ref_frame->getPacketSize()) {
        qDebug() << "ERROR: Comparison size differs: Received = " << this->getPacketSize() << ", expected " << ref_frame->getPacketSize();
        return false;
    }

    //this->print_frame("Received");

    //ref_frame->print_frame("Reference");

    bool data_error = false;
    for (quint32 idx = 0; idx < ref_frame->getPacketSize(); idx++) {
        if (!(ignore_byte_idx.contains(idx)) && this->getByte(idx) != ref_frame->getByte(idx)) {
            qDebug() << "ERROR in data " << idx << ": Received 0x" << QString::number(static_cast<quint8>(this->getByte(idx)), 16) << ", expected 0x" << QString::number(static_cast<quint8>(ref_frame->getByte(idx)), 16);
            data_error = true;
            //return false;
        }
    }

    if (data_error)
        return false;

    return true;
}
