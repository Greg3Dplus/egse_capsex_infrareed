#ifndef CU_FRAME_H
#define CU_FRAME_H

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QIODevice>
#include <QDataStream>
#include <QPixmap>

#include "qpixarray.h"
#include "print_data.h"
#include <QFileInfo>

#include "prospect_regs.h"
#include "niimaq.h"
#include "cameralink.h"

#define HEADER_SIZE     29
#define MAX_WIDTH_IMX253   4112
#define MAX_HEIGHT_IMX253  3008
#define MAX_WIDTH_IMX990   1392
#define MAX_HEIGHT_IMX990  1024


#define MAX_WIDTH_CMV   2048
#define MAX_HEIGHT_CMV  2048
#define NB_PXL_PER_CC   8
#define DEBUG         0


#define DEBUG         0

class CU_Frame : public QObject
{
    Q_OBJECT
public:
    CU_Frame(quint8 nb_bytes_per_pixel, quint8 cam_fw_type);
    CU_Frame(CU_Frame* input_frame);
    CU_Frame(void* header_ar, quint8 nb_bytes_per_pixel, quint8 cam_fw_type, quint8 nb_bits);

    CU_Frame(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file
             quint8 cam_fw_type, quint8 nb_bits_final,

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
             quint8 binning_X   , quint8 binning_Y, quint8 binmode,
             quint8 nb_bits_sensor     ,    // Shall be 10 or 12
             quint8 test_mode   ,
             quint32 integration_time,
             quint16 ADC_gain  ,
             quint16 temperature,
             quint8 EOS       );

    void init_CU_Frame(quint8 nb_bytes_per_pixel, quint8 cam_fw_type);
    void setNbBits(quint8 nb_bits);
    void setNbBytes_per_pix(quint8 nb_bytes_per_pixel);
    void setCamType(quint8 cam_fw_type);
    void setCMVHeaderSize(quint8 CMV_HEADER_SIZE);

    void set_ref_height(quint16 ref_height);
    bool init_pkt(void* ImaqBuffe);
    //void Byte_to_pix(quint32 size_pix, QByteArray byte, quint16 *pix, quint8 nb_bits);
    //void Byte_to_pix(quint32 size_pix, QByteArray byte, quint16 *pix, quint8 nb_bits, quint32 offset);
    quint8 getHeader_field(quint16 num_field);
    quint8 getFooter_field(quint16 num_field);
    quint8 getByte(quint32 address);
    quint32 getData(quint32 address, quint8 nb_bits);
    quint32 getData(quint32 address, quint8 nb_bits, bool no_shift);

    quint32 getPacketSize(void);
    quint32 getPacketLength(void);
    quint32 compute_ref_PktLength(void);
    quint32 getExp_reg(void);
    quint16 getGain(void);
    quint16 getWidth(void);
    quint16 getHeight(void);
    quint16 getPktHeight(void);
    quint16 getOffset_x(void);
    quint16 getOffset_y(void);
    quint16 getTemp_reg(void);
    double  getTemp_val(double cmv_in_freq);
    quint32 getPixel(quint16 num_line, quint16 num_pix, quint8 nb_bits);
    quint32 getPixelRGB(quint16 num_line, quint16 num_pix, quint8 nb_bits);
    QTime*   getTime(void);
    quint8  getFrame_ID(void);
    quint8  getFooter_size(void);
    quint8  getNb_bits(void);
    quint8  getCMVHeaderSize(void);
    quint8  getBinningX();
    QString  getFrame_tag (void);
    quint32  getPixSum(void);
    quint32  getBlackCount(void);
    quint32  getWhiteCount(void);
    quint32  getCRC(void);
    bool     isEOS(void);
    void* getPacket(void);
    QPixmap toPixMap(quint8 color);
    void     create_txt_file (QString file_name,
                              double cmv_in_freq, quint16 ADC_gain, quint8 vramp1, quint8 vramp2, quint16 offset, quint8 fot_length, quint8 cam_fw_type, double H_Period);

    quint32 compute_ref_PktSize(quint16 width, quint16 height, quint8 footer_size);
    quint32 compute_ref_PktSize(quint16 width, quint16 height, quint8 nb_bytes, quint8 CMV_HEADER_SIZE, quint8 footer_size);

    QPixArray header_footer(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file

                            quint16 pix_start   ,
                            quint16 pix_last    ,
                            quint16 row_start   ,
                            quint16 nb_row      ,

                            quint16 black_lim ,
                            quint16 white_lim ,

                            quint8 nb_bytes    ,
                            QByteArray frame_tag   ,    // Array of 8 Bytes
                            quint8 footer      ,   // 0 or 1
                            quint8 binning_X    , quint8 binning_Y, quint8 binmode,
                            quint8 nb_bits_sensor     ,    // Shall be 10 or 12
                            quint8 test_mode       ,
                            quint32 integration_time,
                            quint16 ADC_gain        ,
                            quint8 frame_idx       ,
                            quint16 temperature,
                            quint8 EOS_flag             );

    QByteArray map_video_pkt(QPixArray picture_data,  // Array of pixels. If defined, we use these values instead of input file
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
                             quint8 ADC_gain);

    QPixArray map_header(quint8 frame_idx       ,
                         quint8 EOS_flag             ,
                         quint8 nb_bits_sensor,
                         quint8 nb_bytes        ,
                         quint8 test_mode       ,
                         quint16 pix_start       ,
                         quint16 pix_last        ,
                         quint8 footer          ,
                         quint8 binning_X         , quint8 binning_Y,
                         QByteArray frame_tag       ,     // Array of 8 Bytes
                         quint16 row_start       ,
                         quint16 nb_row          ,  // Number of lines
                         quint32 integration_time,
                         quint16 temperature,
                         quint16 ADC_gain);

    quint32 binning(QPixArray data, quint8 debug, quint8 binmode);

    QPixArray map_data(QPixArray data, quint8 SOF_flag, quint8 EOL_flag, quint8 EOF_flag, quint8 EOS_flag, quint8 h_f, quint8 nb_bits);
    QPixArray map_footer (quint32 pix_sum, quint32 nb_dark, quint32 nb_white);
    QPixArray map_crc(QPixArray data_16b, quint8 EOS_flag);
    quint32 calc_crc(quint8 data_in, quint32 fcs);
    void print_frame(void);
    void print_frame(QString frame_name);

    void copyCU_Frame(CU_Frame* input_frame);
    bool compare(CU_Frame* ref_frame, QVector<quint32> ignore_byte_idx);


private:
    Int8*  packet;      // Full packet, including header
    QTime *time;
    quint8 nb_bytes_per_pixel;
    quint8 CMV_HEADER_SIZE;
    quint8 nb_bits;
    quint8 cam_fw_type;
    quint16 ref_height;  // For 3DIPCC0802, the height is not computed inside firmware if the binning Y is not a power of 2. Therefore, it shall be an input of CU_Frame
    quint8 binning_x;
signals:
    void CU_error();
};

#endif // CU_FRAME_H
