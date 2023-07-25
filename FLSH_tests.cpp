
#include "streamingthreads.h"


/**
* Error detection during PROGRAM (timeout and operation failed)
*/
quint8 StreamingThreads:: FLSH_001(QString file_name)
{
    QFile outputFile(file_name);

    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_001 ###";
        out << "\n";

        quint32 cor_vec = 0x032211;

        quint8 nb_data = 3;

        QByteArray rd_cor_vec;

        send_TC(FLASH_CTL_ba | APB_COR_VECT0, ((cor_vec >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_COR_VECT1, ((cor_vec >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_COR_VECT2, (cor_vec & 0xFF), this->tmtc_channel);

        rd_cor_vec.clear();
        for (int i = 0; i < nb_data; i++){
            rd_cor_vec.append((char)send_TM(FLASH_CTL_ba |APB_COR_VECT0 + i, this->tmtc_channel));
        }
        print_data_Bytes(rd_cor_vec);
        qDebug() << "-----------";

        rd_cor_vec.clear();
        rd_cor_vec = send_TM(nb_data, FLASH_CTL_ba |APB_COR_VECT0 , this->tmtc_channel);
        print_data_Bytes(rd_cor_vec);

        emit Pause();
        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
        }

        /**********************/
        quint8 nb_chan = 4;
        /**********************/

        this->config_CU(0,  // binning
                        1,        // test_mode
                        59,
                        10,  // nb_bits
                        0,
                        10000.0,
                        20,
                        nb_chan, // nb_chan
                        this->tmtc_channel);  //

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);

        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, (bypass_flash << 3) | 6, this->tmtc_channel);
        quint8 spec_op_sts;

        quint8 mode_reg_sts = send_TM(FLASH_CTL_ba |APB_MODE_REG , this->tmtc_channel);
        print_log("mode_reg_sts = 0x" + QString::number(mode_reg_sts,16), out);

        // Check Bypass mode
        if( (mode_reg_sts >> 3) == 1){
            print_log("Bypass mode ENABLED!", out);
        }
        
        // FFB Operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);

        sleep(1);

        quint8 version = send_TM(FLASH_CTL_ba |APB_VERSION_STS , this->tmtc_channel);
        if (version != MCFMC_VERSION) {
            print_log("ERROR version", out);
        }

        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }

        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
            
        quint32 block_fail;
        quint32 blkdiv8;
        int block_fail_sts;
         // print_log("debut bad block", out);
        QByteArray BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS , this->tmtc_channel);
        bool block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 256) + 1;
              // print_log("block_fail : " + QString::number(block_fail,10), out);
            blkdiv8     = floor(block_fail/8);
             // print_log("blkdiv8 : " + QString::number(blkdiv8,10), out);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
             // print_log("block_fail_sts : " + QString::number(block_fail_sts,10), out);
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
                 // print_log("bad block trouver", out);
            }
        }
        //block_fail  = 35;
        quint8 page_fail = rand() % 64;
        
        print_log("Block fail is " + QString::number(block_fail,10), out);
        print_log("Page fail is " + QString::number(page_fail,10), out);
        
        quint32 address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_FAIL, this->tmtc_channel);
    
        print_log("Request video", out);
        quint8  nb_image = 8;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000    ,   // int interval
                          this->tmtc_channel);
        int num_image = 0;
        while (num_image < nb_image) {
            print_log("Wait packet", out);
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        
        //QByteArray BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);

        QByteArray BLK_ERR_STS;
        BLK_ERR_STS.clear();
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel));
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS1 , this->tmtc_channel));
        print_data_Bytes(BLK_ERR_STS);

        // BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        // print_data_Bytes(BLK_ERR_STS);

        quint8 op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        quint8 timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        quint8 op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        quint16 blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        print_log("blk_error_sts 0 =" + QString::number(BLK_ERR_STS.at(0),10), out);
        print_log("blk_error_sts 1 =" + QString::number(BLK_ERR_STS.at(1),10), out);
        print_data_Bytes(BLK_ERR_STS);
        print_log("op_type 1 " + QString::number(op_type,10), out);
        if (op_type != 0x02 ) {
            print_log("ERROR operation detected to fail is not a program operation", out);
        }
        if (timeout == 0x01 ) {
            print_log("ERROR timeout detected", out);
        }
        if (op_failed != 0x01 ) {
            print_log("ERROR operation fail not detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        //BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        // print_log("blk_error_sts 0 " + QString::number(BLK_ERR_STS.at(0),10), out);
        // print_log("blk_error_sts 1 " + QString::number(BLK_ERR_STS.at(1),10), out);

        BLK_ERR_STS.clear();
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel));
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS1 , this->tmtc_channel));


        quint8 blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
        quint8 blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        print_log("blk_sts " + QString::number(blk_sts,10), out);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
        
        
        
        BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS + 32, this->tmtc_channel);
        block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 256) + 256;
            blkdiv8     = floor((block_fail-256)/8);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
        //block_fail  = 26;
        page_fail = rand() % 64;
        
        print_log("Block fail is " + QString::number(block_fail,10), out);
        print_log("Page fail is " + QString::number(page_fail,10), out);
        
        address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_TIMEOUT, this->tmtc_channel);
        
       // quint32 cnt_timeout = 750000/20;
       quint32 cnt_timeout = 750000/15.625;
        
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
    
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        num_image = 0;
        while (num_image < nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture1_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        
        //BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        BLK_ERR_STS.clear();
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel));
        BLK_ERR_STS.append((char)send_TM(FLASH_CTL_ba |APB_BLOCKERR_STS1 , this->tmtc_channel));
        print_data_Bytes(BLK_ERR_STS);
        op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        print_log("blk_error_sts 0 =" + QString::number(BLK_ERR_STS.at(0),10), out);
        print_log("blk_error_sts 1 =" + QString::number(BLK_ERR_STS.at(1),10), out);
        print_data_Bytes(BLK_ERR_STS);
        print_log("op_type 1 " + QString::number(op_type,10), out);
        print_log("blknum 1 " + QString::number(((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF),10), out);
        if (op_type != 0x02 ) {
            print_log("ERROR operation detected to fail is not a program operation", out);
        }
        if (timeout != 0x01 ) {
            print_log("ERROR timeout not detected", out);
        }
        if (op_failed == 0x01 ) {
            print_log("ERROR operation fail detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + 32 + blkdiv8, this->tmtc_channel);
        blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Error detection during READ (timeout only)
*/
quint8 StreamingThreads:: FLSH_002(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_002 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        
        quint8 spec_op_sts;
        
        // Add Bad Block Operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
            
        quint32 block_fail;
        quint32 blkdiv8;
        quint8 block_fail_sts;
        QByteArray BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS , this->tmtc_channel);
        bool block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 256) + 1;
            blkdiv8     = floor(block_fail/8);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
            
        quint8 page_fail = rand() % 64;
        
        print_log("Block fail is " + QString::number(block_fail,10), out);
        print_log("Page fail is " + QString::number(page_fail,10), out);
        
        quint32 address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, READ_OP_TIMEOUT, this->tmtc_channel);
    
        // quint32 cnt_timeout = 30000/20;
        quint32 cnt_timeout = 30000/15.625;
        
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
    
    
        quint8  nb_image = 8;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          2000       ,   // int interval
                          this->tmtc_channel);
        int num_image = 0;
        while (num_image < nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        
        QByteArray BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        quint8 op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        quint8 timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        quint8 op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        quint16 blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        //print_log("blk_error_sts 0 " + QString::number(BLK_ERR_STS.at(0),10), out);
        //print_log("blk_error_sts 1 " + QString::number(BLK_ERR_STS.at(1),10), out);
        print_log("op_type " + QString::number(op_type,10), out);
        print_log("timeout " + QString::number(timeout,10), out);
        print_log("op_failed " + QString::number(op_failed,10), out);
        print_log("blknum " + QString::number(((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF),10), out);

        if (op_type != 0x01 ) {
            print_log("ERROR operation detected to fail is not a read operation", out);
        }
        if (timeout == 0x00 ) {
            print_log("ERROR timeout not detected", out);
        }
        if (op_failed == 0x01 ) {
            print_log("ERROR operation fail detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        quint8 blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
        quint8 blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Error detection during ERASE (timeout and operation failed)
*/
quint8 StreamingThreads:: FLSH_003(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_003 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        
        quint8 spec_op_sts;
        
        // Add Bad Block Operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
            
        quint32 block_fail;
        quint32 blkdiv8;
        quint8 block_fail_sts;
        QByteArray BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS , this->tmtc_channel);
        bool block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 128) + 1;
            blkdiv8     = floor(block_fail/8);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
            
        quint8 page_fail = rand() % 64;
        
        print_log("Block to fail is " + QString::number(block_fail,10), out);
        print_log("Page to fail is " + QString::number(page_fail,10), out);
        
        quint32 address_fail = (block_fail << 6) | page_fail;
        qDebug() << "address fail = 0x" + QString::number(address_fail,16);
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);

        QByteArray reg_addr_fail;
        reg_addr_fail.clear();
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL2_REG , this->tmtc_channel));
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL1_REG , this->tmtc_channel));
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL0_REG , this->tmtc_channel));
        qDebug() << "reg_addr_fail = ";
        print_data_Bytes(reg_addr_fail);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, ERASE_OP_FAIL, this->tmtc_channel);
    
        quint8  nb_image = 4;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(10);
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        int num_image = 0;
        while (num_image < 2*nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        //sleep(10);

        QByteArray  ErasePnt = send_TM(2, FLASH_CTL_ba |APB_POINTER_ERASE0_STS , this->tmtc_channel);
        quint32 erasepnt_all  = ((ErasePnt.at(0) & 0x0F) << 8) | (ErasePnt.at(1) & 0xFF);
        print_log("erase pointer" + QString::number(erasepnt_all,10), out);

        QByteArray BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        quint8 op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        quint8 timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        quint8 op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        quint16 blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        print_log("op_type " + QString::number(op_type,10), out);
        print_log("timeout " + QString::number(timeout,10), out);
        print_log("op_failed " + QString::number(op_failed,10), out);
        print_log("blknum " + QString::number(((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF),10), out);
        if (op_type != 0x03 ) {
            print_log("ERROR operation detected to fail is not a erase operation", out);
        }
        if (timeout == 0x01 ) {
            print_log("ERROR timeout detected", out);
        }
        if (op_failed != 0x01 ) {
            print_log("ERROR operation fail not detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        quint8 blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
        quint8 blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        print_log("blk_sts    = " + QString::number(blk_sts,10) ,out);
        print_log("block_fail = " + QString::number(block_fail,10) ,out);
        print_log("blkdiv8    = " + QString::number(blkdiv8,10) ,out);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
        
        QByteArray WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        quint32 writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | ((WritePnt.at(1) & 0xFF) << 8) | (WritePnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        
        BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS + 32, this->tmtc_channel);
        block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 128) + 256;
            blkdiv8     = floor(block_fail/8);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
            
        page_fail = rand() % 64;
        
        print_log("Block to fail is " + QString::number(block_fail,10), out);
        print_log("Page to fail is " + QString::number(page_fail,10), out);
        
        address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);

        reg_addr_fail.clear();
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL2_REG , this->tmtc_channel));
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL1_REG , this->tmtc_channel));
        reg_addr_fail.append(send_TM(FLASH_CTL_ba |APB_ADDR_FAIL0_REG , this->tmtc_channel));
        qDebug() << "reg_addr_fail = ";
        print_data_Bytes(reg_addr_fail);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_TIMEOUT, this->tmtc_channel);
    
        // quint32 cnt_timeout = 4500000/20;
        quint32 cnt_timeout = 750000/15.625;
        
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
    
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(10);
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        num_image = 0;
        while (num_image < 2*nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture1_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        //sleep(10);

        ErasePnt = send_TM(2, FLASH_CTL_ba |APB_POINTER_ERASE0_STS , this->tmtc_channel);
        erasepnt_all  = ((ErasePnt.at(0) & 0x0F) << 8) | (ErasePnt.at(1) & 0xFF);
        print_log("erase pointer" + QString::number(erasepnt_all,10), out);

        BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        print_log("op_type " + QString::number(op_type,10), out);
        print_log("timeout " + QString::number(timeout,10), out);
        print_log("op_failed " + QString::number(op_failed,10), out);
        print_log("blknum " + QString::number(((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF),10), out);
        if (op_type != 0x02 ) {
            print_log("ERROR operation detected to fail is not a program operation", out);
        }
        if (timeout != 0x01 ) {
            print_log("ERROR timeout not detected", out);
        }
        if (op_failed == 0x01 ) {
            print_log("ERROR operation fail detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
        blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        print_log("blk_sts    = " + QString::number(blk_sts,10) ,out);
        print_log("block_fail = " + QString::number(block_fail,10) ,out);
        print_log("blkdiv8    = " + QString::number(blkdiv8,10) ,out);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }


        WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | ((WritePnt.at(1) & 0xFF) << 8) | (WritePnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);


        // QByteArray BB_table;
        // BB_table.clear();
        // for (int i=0; i <= 3; i++) {
        //     BB_table.append(send_TM(128, FLASH_CTL_ba | APB_BLOCK_VALID_STS +128*i , this->tmtc_channel));
        // }
        // print_data_Bytes(BB_table);


    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* READ_NOMINAL functionnality verification
*/
quint8 StreamingThreads:: FLSH_004(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_004 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }

        quint8  nb_image = 2;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(2);
        nb_image = 2;
        width    = 1024;
        height   = 1024;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);

                          
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        msleep(1);
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
                          
        for (int i = 0; i< 2; i++) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(i) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
        }

        print_log("received first sequence", out);
        ReceiveOperation receiveOp;
        Packet *pReceivedPacket;
        TransferOperation::TransferStatus status;
        quint32 receiveBufferLength;
        StreamItem *pStreamItem;
        
        if (!receiveOp.CreateRxOperation(1, ReceiveOperation::RECEIVE_PACKETS))
        {
            qDebug () << "Couldn't create receive operation, exiting.";
            this->vid_channel.CloseChannel();
            return {};
        }
        this->vid_channel.SubmitTransferOperation(&receiveOp);
        print_log("before wait", out);
        status = receiveOp.WaitOnTransferOperationCompletion(5000);
        print_log("after wait", out);
        if (status == TransferOperation::TRANSFER_STATUS_COMPLETE)
        {
            qDebug () << "Error packet received while Read enable is set to 0" << status;
        }
        print_log("before GetTransferItem", out);
        pStreamItem = receiveOp.GetTransferItem(0);
        print_log("after GetTransferItem", out);
        pReceivedPacket = (Packet*)pStreamItem;
        print_log("before receiveBufferLength", out);
        /*pReceivedPacket->GetPacketData(&receiveBufferLength);
        if (receiveBufferLength != 0)
        {
            qDebug () << "Error a byte have been received" << status;
        }
        */
        print_log("after receiveBufferLength", out);
        send_TC(FLASH_CTL_ba | APB_POWER_CTL_REG, 1, this->tmtc_channel);

        sleep(1);


        quint8 power_off_sts = send_TM(FLASH_CTL_ba | APB_POWER_STS, this->tmtc_channel);
        if (power_off_sts & 0x01 != 1) {
            print_log("ERROR mcfmc not ready for power off", out);
        }
        else {
            qDebug () << "ready for power off\n";
        }
        emit Pause();
        
        this->tmtc_channel.CloseChannel();
        this->vid_channel.CloseChannel();

        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
            //qDebug () << "IN PAUSE";
        }
        qDebug () << "EXIT!";

        // SPW_interface DeviceToUse;
        // DeviceToUse.find_SPW_Device();
        // DeviceToUse.ResetDevice();
        //
        // const quint8 tmtc_channelNumber = 1;
        // const quint8 vid_channelNumber = 2;
        // DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);
        //
        // qDebug() << "Opening SPW channel...";
        // this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        // this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);

        // Check if channel was previously not closed
        if(this->tmtc_channel.IsOpen() == 1) {  // return 1, if the channel is open, otherwise 0.
            qDebug() << "TM/TC Channel was found opened!";
        }
        else {
            qDebug() << "TM/TC Channel " + QString::number(tmtc_channel.GetChannelNumber(), 10) + " already closed...";
        }

        SPW_interface DeviceToUse;
        DeviceToUse.find_SPW_Device();
        DeviceToUse.ResetDevice();

        DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);

        qDebug() << "Opening SPW channel...";
        this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);
        
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        
        
        for (int i = 0; i< 2; i++) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture1_" + QString::number(i) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
        }
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 64, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR erase block 0 not performed", out);
        }

    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* READ_CONFIRM functionnality verification
*/
quint8 StreamingThreads:: FLSH_005(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_005 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 1, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
        QByteArray WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        quint32 writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        QByteArray ReadCurPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        quint32 readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        QByteArray ReadLastPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        quint32 readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);
        //send_TC(FLASH_CTL_ba | APB_MODE_REG, 1, this->tmtc_channel);

        quint8  nb_image = 2;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(2);
        nb_image = 2;
        width    = 1024;
        height   = 1024;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(10);


        QByteArray NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        quint32 nb_sequence = (NAND_STATUS.at(0) & 0x03) << 16 |(NAND_STATUS.at(1) & 0x03) << 8 | NAND_STATUS.at(2);

        print_log("nb sequences" + QString::number(nb_sequence,10), out);

        WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        ReadCurPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        ReadLastPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 16, this->tmtc_channel);

        for (int i = 0; i< 2; i++) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(i) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
        }
        WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        ReadCurPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        ReadLastPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);
        ReceiveOperation receiveOp;
        Packet *pReceivedPacket;
        TransferOperation::TransferStatus status;
        quint32 receiveBufferLength;
        StreamItem *pStreamItem;
        quint8 *pReceiveBuffer;
        
        if (!receiveOp.CreateRxOperation(1, ReceiveOperation::RECEIVE_PACKETS))
        {
            qDebug () << "Couldn't create receive operation, exiting.";
            /* clean up resources created thus far, prior to exiting function */
            this->vid_channel.CloseChannel();
            return {};
        }
        /* start receiving a packet */
        this->vid_channel.SubmitTransferOperation(&receiveOp);
        /* wait indefinitely on a packet being received */
        status = receiveOp.WaitOnTransferOperationCompletion(5000);
        if (status == TransferOperation::TRANSFER_STATUS_COMPLETE)
        {
            qDebug () << "Error packet received while Read enable is set to 0" << status;
        }
        
        /* get the stream item received */
        pStreamItem = receiveOp.GetTransferItem(0);
        pReceivedPacket = (Packet*)pStreamItem;
        /*pReceivedPacket->GetPacketData(&receiveBufferLength);
        if (receiveBufferLength != 0)
        {
            qDebug () << "Error a byte have been received" << status;
        }*/
        
        
        send_TC(FLASH_CTL_ba | APB_POWER_CTL_REG, 1, this->tmtc_channel);

        sleep(1);


        quint8 power_off_sts = send_TM(FLASH_CTL_ba | APB_POWER_STS, this->tmtc_channel);
        if (power_off_sts & 0x01 != 1) {
            print_log("ERROR mcfmc not ready for power off", out);
        }
        else {
            qDebug () << "ready for power off\n";
        }
        
        emit Pause();

        this->tmtc_channel.CloseChannel();
        this->vid_channel.CloseChannel();

        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
            //qDebug () << "IN PAUSE";
        }
        qDebug () << "EXIT!";

        SPW_interface DeviceToUse;
        DeviceToUse.find_SPW_Device();
        DeviceToUse.ResetDevice();

        const quint8 tmtc_channelNumber = 1;
        const quint8 vid_channelNumber = 2;
        DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);

        qDebug() << "Opening SPW channel...";
        this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);
        
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 1, this->tmtc_channel);
        
        WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        ReadCurPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        ReadLastPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);

        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 32, this->tmtc_channel);
        QByteArray video_pkt = waitPacket(this->vid_channel);
        (*this->rcv_data_Byte)->format_header(video_pkt);
        emit FrameReceived();
        float cmv_in_freq = 6.4;
        QString path ="./";
        QDir dir;
        path ="./DEMO/";
        if(!dir.exists(path))
            { dir.mkpath(path);}
        (*this->rcv_data_Byte)->create_txt_file(path + "picture1_3.txt",  // "image_test.txt",
                                                    cmv_in_freq);

        video_pkt = waitPacket(this->vid_channel);
        (*this->rcv_data_Byte)->format_header(video_pkt);
        emit FrameReceived();
        (*this->rcv_data_Byte)->create_txt_file(path + "picture1_4.txt",  // "image_test.txt",
                                                    cmv_in_freq);
        
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 16, this->tmtc_channel);
        video_pkt = waitPacket(this->vid_channel);
        (*this->rcv_data_Byte)->format_header(video_pkt);
        emit FrameReceived();
        (*this->rcv_data_Byte)->create_txt_file(path + "picture1_5.txt",  // "image_test.txt",
                                                    cmv_in_freq);

        video_pkt = waitPacket(this->vid_channel);
        (*this->rcv_data_Byte)->format_header(video_pkt);
        emit FrameReceived();
        (*this->rcv_data_Byte)->create_txt_file(path + "picture1_6.txt",  // "image_test.txt",
                                                    cmv_in_freq);
                                                    
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 64, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR erase block 0 not performed", out);
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify Fetch Failing Block operation
*/
quint8 StreamingThreads:: FLSH_006(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_006 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
        
        quint32 block_fail;
        quint32 blkdiv8;
        int block_fail_sts;
        quint8 BLOCK_STATUS[512];
        for(int i = 0; i < 512; i++){
            BLOCK_STATUS[i] = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + i, this->tmtc_channel);
        }
        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 8; j++){
                if (((BLOCK_STATUS[i] >> j) & 0x01 ) == 1){
                    print_log("block " + QString::number(8*i+j,10) + "  invalide", out);
                }
            }
        }


        bool block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 4095) + 1;
            blkdiv8     = floor(block_fail/8);
            block_fail_sts = BLOCK_STATUS[blkdiv8] >> (block_fail % 8) & 0x01;
            print_log("block_fail =" + QString::number(block_fail,10), out);
            print_log("blkdiv8 =" + QString::number(blkdiv8,10), out);
            print_log("block_fail_sts =" + QString::number(block_fail_sts,10), out);
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
            
        quint8 page_fail = rand() % 2;
        print_log("block corrupted " + QString::number(block_fail,10), out);
        print_log("page corrupted " + QString::number(page_fail,10), out);
        
        quint32 address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, FFB_FAIL, this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        quint8 BLOCK_STATUS2[512];
        for(int i = 0; i < 512; i++){
            BLOCK_STATUS2[i] = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + i , this->tmtc_channel);
        }
        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 8; j++){
                if (((BLOCK_STATUS2[i] >> j) & 0x01 ) == 1){
                    print_log("block " + QString::number(8*i+j,10) + " invalide", out);
                }
            }
        }
        
        //if ((BLOCK_STATUS2[blkdiv8] | (1 << (block_fail % 8)) ) != BLOCK_STATUS[blkdiv8]) {
        if (BLOCK_STATUS2[blkdiv8] != BLOCK_STATUS[blkdiv8]) {
            print_log("Error block forced to be fail not detected by FFB", out);
        }
                
        
        
        block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 4095) + 1;
            blkdiv8     = floor(block_fail/8);
            block_fail_sts = BLOCK_STATUS[blkdiv8] >> (block_fail % 8) & 0x01;
            print_log("block_fail =" + QString::number(block_fail,10), out);
            print_log("blkdiv8 =" + QString::number(blkdiv8,10), out);
            print_log("block_fail_sts =" + QString::number(block_fail_sts,10), out);
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
            
        page_fail = rand() % 2;
        
        address_fail = (block_fail << 6) | page_fail;
        print_log("block corrupted " + QString::number(block_fail,10), out);
        print_log("page corrupted " + QString::number(page_fail,10), out);
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        
        quint8 BLOCK_STATUS3[512];
        for(int i = 0; i < 512; i++){
            BLOCK_STATUS3[i] = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + i , this->tmtc_channel);
        }
        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 8; j++){
                if (((BLOCK_STATUS3[i] >> j) & 0x01 ) == 1){
                    print_log("block " + QString::number(8*i+j,10) + " invalide", out);
                }
            }
        }


        //if ((BLOCK_STATUS3[blkdiv8] | (1 << (block_fail % 8)) ) != BLOCK_STATUS[blkdiv8]) {
        if (BLOCK_STATUS3[blkdiv8] != BLOCK_STATUS[blkdiv8]) {
            print_log("Error block forced to be fail not detected by FFB", out);
        }
                
        /*for (int i = 1; i < 4096; i++){
            if (i != blkdiv8) {
                if (BLOCK_STATUS[i] != BLOCK_STATUS3[i]) {
                    print_log("Error result of FFB not the same", out);
                }
            }
        }*/
        
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify Add Bad Block operation
*/
quint8 StreamingThreads:: FLSH_007(QString file_name)
{
    QFile outputFile(file_name);

    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_007 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);

        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        quint8 spec_op_sts;
        
        // FFB Operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);

        sleep(1);

        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }

        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
            
        quint32 block_fail;
        quint32 blkdiv8;
        int block_fail_sts;
        // print_log("debut bad block", out);
        QByteArray BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS , this->tmtc_channel);
        bool block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 256) + 1;
            // print_log("block_fail : " + QString::number(block_fail,10), out);
            blkdiv8     = floor(block_fail/8);
            // print_log("blkdiv8 : " + QString::number(blkdiv8,10), out);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            // print_log("block_fail_sts : " + QString::number(block_fail_sts,10), out);
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
                // print_log("bad block trouver", out);
            }
        }
        //block_fail  = 35;
        quint8 page_fail = rand() % 64;
        
        print_log("Block fail is " + QString::number(block_fail,10), out);
        print_log("Page fail is " + QString::number(page_fail,10), out);
        
        quint32 address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_FAIL, this->tmtc_channel);
    
        quint8  nb_image = 8;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        int num_image = 0;
        while (num_image < nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        
        QByteArray BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        quint8 op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        quint8 timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        quint8 op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        quint16 blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        // print_log("blk_error_sts 0 " + QString::number(BLK_ERR_STS.at(0),10), out);
        // print_log("blk_error_sts 1 " + QString::number(BLK_ERR_STS.at(1),10), out);
        // print_log("op_type 1 " + QString::number(op_type,10), out);
        if (op_type != 0x02 ) {
            print_log("ERROR operation detected to fail is not a program operation", out);
        }
        if (timeout == 0x01 ) {
            print_log("ERROR timeout detected", out);
        }
        if (op_failed != 0x01 ) {
            print_log("ERROR operation fail not detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        // print_log("blk_error_sts 0 " + QString::number(BLK_ERR_STS.at(0),10), out);
        // print_log("blk_error_sts 1 " + QString::number(BLK_ERR_STS.at(1),10), out);


        quint8 blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
        quint8 blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        print_log("blk_sts " + QString::number(blk_sts,10), out);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
        
        
        
        BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS + 32, this->tmtc_channel);
        block_corrupt_found = false;
        while (block_corrupt_found == false) {
            block_fail  = (rand() % 256) + 256;
            blkdiv8     = floor((block_fail-256)/8);
            block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
            if (block_fail_sts == 0) {
                block_corrupt_found = true;
            }
        }
        //block_fail  = 26;
        page_fail = rand() % 64;
        
        // print_log("Block fail is " + QString::number(block_fail,10), out);
        // print_log("Page fail is " + QString::number(page_fail,10), out);
        
        address_fail = (block_fail << 6) | page_fail;
        
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_TIMEOUT, this->tmtc_channel);
        
        // quint32 cnt_timeout = 750000/20;
        quint32 cnt_timeout = 750000/15.625;
        
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
    
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        num_image = 0;
        while (num_image < nb_image) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();

            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture1_" + QString::number(num_image) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);
            num_image++;
        }
        
        BLK_ERR_STS = send_TM(2, FLASH_CTL_ba |APB_BLOCKERR_STS0 , this->tmtc_channel);
        op_type     = (BLK_ERR_STS.at(0) >> 6) & 0x03;
        timeout     = (BLK_ERR_STS.at(0) >> 5) & 0x01;
        op_failed   = (BLK_ERR_STS.at(0) >> 4) & 0x01;
        blknum      = ((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF);
        // print_log("blk_error_sts 0 " + QString::number(BLK_ERR_STS.at(0),10), out);
        // print_log("blk_error_sts 1 " + QString::number(BLK_ERR_STS.at(1),10), out);
        // print_log("op_type 1 " + QString::number(op_type,10), out);
        // print_log("blknum 1 " + QString::number(((BLK_ERR_STS.at(0) & 0x0F) << 8) | (BLK_ERR_STS.at(1) & 0xFF),10), out);
        if (op_type != 0x02 ) {
            print_log("ERROR operation detected to fail is not a program operation", out);
        }
        if (timeout != 0x01 ) {
            print_log("ERROR timeout not detected", out);
        }
        if (op_failed == 0x01 ) {
            print_log("ERROR operation fail detected", out);
        }
        if (blknum != block_fail ) {
            print_log("ERROR block detected as failed is not good", out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 2, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after addbb", out);
        }
        
        blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + 32 + blkdiv8, this->tmtc_channel);
        blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
        if (blk_sts_list == 0) {
            print_log("ERROR block not added in Bad block list after addbb op", out);
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify FORMAT, full flag and sequence counter
*/
quint8 StreamingThreads:: FLSH_008(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_008 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }

        quint16 nb_block_valid = 4095;
        for (int i = 0; i < 512; i++) {
            quint8 BLOCK_STATUS = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS +i , this->tmtc_channel);
            //print_log("block " + QString::number(BLOCK_STATUS & 0xFF,10) + " invalide", out);
            for (int j = 0; j < 8; j++){
                if (((BLOCK_STATUS >> j) & 0x01 ) == 1){
                    nb_block_valid--;
                    print_log("block " + QString::number(8*i+j,10) + " invalide", out);
                    msleep(1);
                }
            }
        }
        
        quint32 nb_page_valid = nb_block_valid*64;
        print_log("nb block valid " + QString::number(nb_block_valid,10), out);
        print_log("nb page valid " + QString::number(nb_page_valid,10), out);
        
        
        
        
        quint8  nb_image = 8;
        quint16 width    = 2048;
        quint16 height   = 2048;
        
        quint32 nb_sequence = 0;
        quint8  full = 0;
        
        quint32 nb_page_to_write = nb_page_valid;
        quint32 page_8x2048x2048 = 16384+1;
        quint32 page_4x2048x2048 = 8192+1;
        quint32 page_2x2048x2048 = 4096+1;
        quint32 page_1x2048x2048 = 2048+1;
        quint32 page_1x1024x1024 = 512+1;
        quint32 page_1x512x512 = 128+1;
        quint32 page_1x256x256 = 32+1;
        quint32 page_1x128x128 = 8+1;
        quint32 page_1x64x64 = 2+1;
        quint32 page_1x32x32 = 1;
        
        quint32 loop_nb = 0;
        
        while (nb_page_to_write != 0) {
            cmv4000_Req_Video(width   ,   // int height,
                              height  ,   // int width,
                              0       ,   // int offset_x,
                              0       ,   // int offset_y,
                              nb_image,   // int nb_image,
                              1000       ,   // int interval
                              this->tmtc_channel);
            //SendVidReq(this->tmtc_channel);
            if (nb_image == 8) {
                sleep(20);
            }
            else if (nb_image == 4) {
                sleep(10);
            }
            else if (nb_image == 2) {
                sleep(5);
            }
            else if (nb_image == 1) {
                sleep(2);
            }


            nb_page_to_write = nb_page_to_write - (nb_image*width*height/2048 + 1);
            print_log("nb_page_to_write" + QString::number(nb_page_to_write,10), out);
            
            
            loop_nb++;
            print_log("loop number " + QString::number(loop_nb,10), out);

            QByteArray NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
            full = (NAND_STATUS.at(0) >> 7) & 0x01;
            nb_sequence = ((NAND_STATUS.at(0) & 0x03) << 16) | ((NAND_STATUS.at(1) & 0xFF) << 8) | (NAND_STATUS.at(2) & 0xFF);
            if ((full == 1) && (nb_page_to_write != 0)) {
                print_log("Error Flash NAND shall not be full", out);
            }
            if ((full == 0) && (nb_page_to_write == 0)) {
                print_log("Error Flash shall be full", out);
            }
            if (loop_nb != nb_sequence) {
                print_log("Error in cnt of sequence.", out);
                print_log("nb sequences" + QString::number(nb_sequence,10), out);
            }
            
            
            QByteArray WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
            quint32 writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | ((WritePnt.at(1) & 0xFF) << 8) | (WritePnt.at(2) & 0xFF);
            print_log("write pointer " + QString::number(writepnt_all,10), out);
            
            
            if (nb_page_to_write >= page_8x2048x2048) {
                nb_image = 8;
                width    = 2048;
                height   = 2048;
            }
            else if (nb_page_to_write >= page_4x2048x2048) {
                nb_image = 4;
                width    = 2048;
                height   = 2048;
            }
            else if (nb_page_to_write >= page_2x2048x2048) {
                nb_image = 2;
                width    = 2048;
                height   = 2048;
            }
            else if (nb_page_to_write >= page_1x2048x2048) {
                nb_image = 1;
                width    = 2048;
                height   = 2048;
            }
            else if (nb_page_to_write >= page_1x1024x1024) {
                nb_image = 1;
                width    = 1024;
                height   = 1024;
            }
            else if (nb_page_to_write >= page_1x512x512) {
                nb_image = 1;
                width    = 512;
                height   = 512;
            }
            else if (nb_page_to_write >= page_1x256x256) {
                nb_image = 1;
                width    = 256;
                height   = 256;
            }
            else if (nb_page_to_write >= page_1x128x128) {
                nb_image = 1;
                width    = 128;
                height   = 128;
            }
            else if (nb_page_to_write >= page_1x64x64) {
                nb_image = 1;
                width    = 64;
                height   = 64;
            }
            else if (nb_page_to_write >= page_1x32x32) {
                nb_image = 1;
                width    = 32;
                height   = 32;
            }
        }
        
        
        print_log("loop number " + QString::number(loop_nb,10), out);
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
        
        
        nb_image = 4;
        width    = 2048;
        height   = 2048;
        
        nb_sequence = 0;
        full = 0;
        
        for (int i = 0; i < 31; i++) {
            cmv4000_Req_Video(width   ,   // int height,
                              height  ,   // int width,
                              0       ,   // int offset_x,
                              0       ,   // int offset_y,
                              nb_image,   // int nb_image,
                              1000       ,   // int interval
                              this->tmtc_channel);
            sleep(10);
            QByteArray NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
            full = (NAND_STATUS.at(0) >> 7) & 0x01;
            nb_sequence = ((NAND_STATUS.at(0) & 0x03) << 16) | ((NAND_STATUS.at(1) & 0xFF) << 8) | (NAND_STATUS.at(2) & 0xFF);
            if (nb_sequence != i+1) {
                print_log("Error in cnt of sequence.", out);
            }
        }
        
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 1, this->tmtc_channel);
        
        for (int i = 0; i < 31; i++) {
            send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 16, this->tmtc_channel);
            print_log("start reading sequence " + QString::number(i,10), out);
            for(int j = 0; j < 4; j++){
                print_log("wait frame " + QString::number(j,10), out);
                QByteArray video_pkt = waitPacket(this->vid_channel);
                (*this->rcv_data_Byte)->format_header(video_pkt);
                emit FrameReceived();

                float cmv_in_freq = 6.4;
                QString path ="./";
                QDir dir;
                path ="./DEMO/";
                if(!dir.exists(path))
                { dir.mkpath(path);}
                

                (*this->rcv_data_Byte)->create_txt_file(path + "picture_" + QString::number(4*i+j) + ".txt",  // "image_test.txt",
                                                        cmv_in_freq);

            }
            QByteArray NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
            full = (NAND_STATUS.at(0) >> 7) & 0x01;
            nb_sequence = ((NAND_STATUS.at(0) & 0x03) << 16) | ((NAND_STATUS.at(1) & 0xFF) << 8) | (NAND_STATUS.at(2) & 0xFF);
            if (full != 1 && i == 0) {
                print_log("Error Flash NAND shall be full", out);
            }
            if (full == 1 && i != 0) {
                print_log("Error Flash NAND shall not be full", out);
            }
            if (nb_sequence != 31-(i+1)) {
                print_log("Error in cnt of sequence.", out);
            }
            
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify SELF TEST operation
*/
quint8 StreamingThreads:: FLSH_009(QString file_name)
{
    QFile outputFile(file_name);

    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_007 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);

        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        quint8 spec_op_sts;
        
        // FFB Operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);

        sleep(1);

        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }

        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
        for (int i = 0; i < 3; i++){
            quint32 block_fail;
            quint32 blkdiv8;
            int block_fail_sts;
            // print_log("debut bad block", out);
            QByteArray BLOCK_STATUS = send_TM(32, FLASH_CTL_ba | APB_BLOCK_VALID_STS , this->tmtc_channel);
            bool block_corrupt_found = false;
            while (block_corrupt_found == false) {
                block_fail  = (rand() % 256) + 1;
                // print_log("block_fail : " + QString::number(block_fail,10), out);
                blkdiv8     = floor(block_fail/8);
                // print_log("blkdiv8 : " + QString::number(blkdiv8,10), out);
                block_fail_sts = BLOCK_STATUS.at(blkdiv8) >> (block_fail % 8) & 0x01;
                // print_log("block_fail_sts : " + QString::number(block_fail_sts,10), out);
                if (block_fail_sts == 0) {
                    block_corrupt_found = true;
                    // print_log("bad block trouver", out);
                }
            }
            //block_fail  = 35;
            quint8 page_fail = rand() % 64;
            
            print_log("Block fail is " + QString::number(block_fail,10), out);
            print_log("Page fail is " + QString::number(page_fail,10), out);
            
            
            quint32 address_fail = (block_fail << 6) | page_fail;
            
            send_TC(FLASH_CTL_ba | APB_ADDR_FAIL0_REG, ((address_fail >> 16) & 0x03), this->tmtc_channel);
            send_TC(FLASH_CTL_ba | APB_ADDR_FAIL1_REG, ((address_fail >> 8) & 0xFF), this->tmtc_channel);
            send_TC(FLASH_CTL_ba | APB_ADDR_FAIL2_REG, (address_fail & 0xFF), this->tmtc_channel);
            
            if (i == 0) {
                send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, WRITE_OP_TIMEOUT, this->tmtc_channel);
                
                // quint32 cnt_timeout = 750000/20;
                quint32 cnt_timeout = 750000/15.625;
                
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
            }
            else if (i == 1) {
                send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, ERASE_OP_FAIL, this->tmtc_channel);
            }
            else if (i == 2) {
                send_TC(FLASH_CTL_ba | APB_OP_FAIL_REG, READ_CORRUPT, this->tmtc_channel);
                
                quint32 cnt_timeout = 4000;
                
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT0_REG, ((cnt_timeout >> 16) & 0x03), this->tmtc_channel);
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT1_REG, ((cnt_timeout >> 8) & 0xFF), this->tmtc_channel);
                send_TC(FLASH_CTL_ba | APB_TIMEOUT_CNT2_REG, (cnt_timeout & 0xFF), this->tmtc_channel);
                
            }

            
            send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 8, this->tmtc_channel);
            
            quint8 tm_st_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
            quint8 st_sts = tm_st_sts >> 3 & 0x01;
            
            int cnt = 0;
            
            while (st_sts == 1) {
                print_log("cnt seconds" + QString::number(cnt,10), out);
                sleep(1);
                tm_st_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
                st_sts = tm_st_sts >> 3 & 0x01;
                cnt = cnt+1;
            }
            
            quint8 blk_sts = send_TM(FLASH_CTL_ba | APB_BLOCK_VALID_STS + blkdiv8, this->tmtc_channel);
            quint8 blk_sts_list = ((blk_sts >> (block_fail % 8)) & 0x01);
            print_log("blk_sts " + QString::number(blk_sts,10), out);
            if (blk_sts_list == 0) {
                print_log("ERROR block not added in Bad block list after addbb op", out);
            }
        }
    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify Erase block 0 operation and power down sequence
*/
quint8 StreamingThreads:: FLSH_0010(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_0010 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }

        quint8  nb_image = 2;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(2);
        nb_image = 2;
        width    = 1024;
        height   = 1024;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);

                          
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 2, this->tmtc_channel);
        msleep(1);
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
                          
        for (int i = 0; i< 2; i++) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();
        }


        print_log("after receiveBufferLength", out);
        send_TC(FLASH_CTL_ba | APB_POWER_CTL_REG, 1, this->tmtc_channel);

        sleep(1);

        QByteArray WritePnt     = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        QByteArray ReadCurPnt   = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        QByteArray ReadLastPnt  = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        QByteArray SeqCnt       = send_TM(3, FLASH_CTL_ba |APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        quint32 writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        quint32 readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        quint32 readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        quint32 seq_cnt_all  = ((SeqCnt.at(0) & 0x03) << 16) | (SeqCnt.at(1) & 0xFF << 8) | (SeqCnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);

        quint8 power_off_sts = send_TM(FLASH_CTL_ba | APB_POWER_STS, this->tmtc_channel);
        if ((power_off_sts & 0x01) != 1) {
            print_log("ERROR mcfmc not ready for power off", out);
        }
        else {
            qDebug () << "ready for power off\n";
        }
        emit Pause();
        
        this->tmtc_channel.CloseChannel();
        this->vid_channel.CloseChannel();

        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
            //qDebug () << "IN PAUSE";
        }
        qDebug () << "EXIT!";

        SPW_interface DeviceToUse;
        DeviceToUse.find_SPW_Device();
        DeviceToUse.ResetDevice();

        const quint8 tmtc_channelNumber = 1;
        const quint8 vid_channelNumber = 2;
        DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);

        qDebug() << "Opening SPW channel...";
        this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);
        
        quint8 init_sts = send_TM(FLASH_CTL_ba |APB_POWER_STS , this->tmtc_channel);
        
        if ((init_sts >> 1) & 0x01 !=  1) {
            print_log("ERROR init not completed", out);
        }
        
        WritePnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        ReadCurPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        ReadLastPnt = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        SeqCnt       = send_TM(3, FLASH_CTL_ba |APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        quint32 writepnt1_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        quint32 readcurpnt1_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        quint32 readlastpnt1_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        quint32 seq_cnt1_all  = ((SeqCnt.at(0) & 0x03) << 16) | (SeqCnt.at(1) & 0xFF << 8) | (SeqCnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);
        
        if (writepnt_all != writepnt1_all) {
            print_log("Error pointer write has changed",out);
        }
        if (readcurpnt_all != readcurpnt1_all) {
            print_log("Error pointer read current has changed",out);
        }
        if (readlastpnt_all != readlastpnt1_all) {
            print_log("Error pointer read last has changed",out);
        }
        
        if (seq_cnt_all != seq_cnt1_all) {
            print_log("Error sequence number has changed",out);
        }
        
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 64, this->tmtc_channel);
        sleep(1);
        spec_op_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR erase block 0 not performed", out);
        }
        
        
        emit Pause();
        
        this->tmtc_channel.CloseChannel();
        this->vid_channel.CloseChannel();

        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
            //qDebug () << "IN PAUSE";
        }
        qDebug () << "EXIT!";

        DeviceToUse.find_SPW_Device();
        DeviceToUse.ResetDevice();

        DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);

        qDebug() << "Opening SPW channel...";
        this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);
        
        init_sts = send_TM(FLASH_CTL_ba |APB_POWER_STS , this->tmtc_channel);
        
        if ((init_sts >> 1) & 0x01 !=  0) {
            print_log("ERROR init shall not be complete", out);
        }
        

    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* ECC functionnalities
*/
quint8 StreamingThreads:: FLSH_0011(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_011 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits
        
        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 6, this->tmtc_channel);
        
        send_TC(FLASH_CTL_ba | APB_COR_VECT2, 64, this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_COR_EN, 1, this->tmtc_channel);
        

        QByteArray errCnt       = send_TM(2, FLASH_CTL_ba |APB_ECC_CNT_REG0 , this->tmtc_channel);
        quint8 overflow         = send_TM(FLASH_CTL_ba |APB_ECC_ERR_STS , this->tmtc_channel);
        quint16 errCnt_all  = ((errCnt.at(0) & 0xFF) << 8) | (errCnt.at(1) & 0xFF);
        print_log("error counter" + QString::number(errCnt_all,10), out);
        print_log("overflow counter" + QString::number(overflow,10), out);

        quint8  nb_image = 4;
        quint16 width    = 2048;
        quint16 height   = 2048;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        
        for (int i = 0; i< nb_image; i++) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            emit FrameReceived();


            float cmv_in_freq = 6.4;
            QString path ="./";
            QDir dir;
            path ="./DEMO/";
            if(!dir.exists(path))
            { dir.mkpath(path);}

            (*this->rcv_data_Byte)->create_txt_file(path + "picture1_" + QString::number(i) + ".txt",  // "image_test.txt",
                                                    cmv_in_freq);


        }
        
        QByteArray WritePnt     = send_TM(3, FLASH_CTL_ba |APB_POINTER_WRITE0_STS , this->tmtc_channel);
        QByteArray ReadCurPnt   = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_CURRENT0_STS , this->tmtc_channel);
        QByteArray ReadLastPnt  = send_TM(3, FLASH_CTL_ba |APB_POINTER_READ_LAST_STSEQ0_STS , this->tmtc_channel);
        QByteArray SeqCnt       = send_TM(3, FLASH_CTL_ba |APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        quint32 writepnt_all  = ((WritePnt.at(0) & 0x03) << 16) | (WritePnt.at(1) & 0xFF << 8) | (WritePnt.at(2) & 0xFF);
        quint32 readcurpnt_all  = ((ReadCurPnt.at(0) & 0x03) << 16) | (ReadCurPnt.at(1) & 0xFF << 8) | (ReadCurPnt.at(2) & 0xFF);
        quint32 readlastpnt_all  = ((ReadLastPnt.at(0) & 0x03) << 16) | (ReadLastPnt.at(1) & 0xFF << 8) | (ReadLastPnt.at(2) & 0xFF);
        quint32 seq_cnt_all  = ((SeqCnt.at(0) & 0x03) << 16) | (SeqCnt.at(1) & 0xFF << 8) | (SeqCnt.at(2) & 0xFF);
        print_log("write pointer " + QString::number(writepnt_all,10), out);
        print_log("read cur pointer" + QString::number(readcurpnt_all,10), out);
        print_log("read last pointer" + QString::number(readlastpnt_all,10), out);
        
        errCnt       = send_TM(2, FLASH_CTL_ba |APB_ECC_CNT_REG0 , this->tmtc_channel);
        overflow         = send_TM(FLASH_CTL_ba |APB_ECC_ERR_STS , this->tmtc_channel);
        errCnt_all  = ((errCnt.at(0) & 0xFF) << 8) | (errCnt.at(1) & 0xFF);
        print_log("error counter" + QString::number(errCnt_all,10), out);
        print_log("overflow counter" + QString::number(overflow,10), out);
        
        send_TC(FLASH_CTL_ba | APB_ECC_ERR_CTL, 1, this->tmtc_channel);
        
        errCnt          = send_TM(2, FLASH_CTL_ba |APB_ECC_CNT_REG0 , this->tmtc_channel);
        overflow        = send_TM(FLASH_CTL_ba |APB_ECC_ERR_STS , this->tmtc_channel);
        errCnt_all      = ((errCnt.at(0) & 0xFF) << 8) | (errCnt.at(1) & 0xFF);
        print_log("error counter" + QString::number(errCnt_all,10), out);
        print_log("overflow counter" + QString::number(overflow,10), out);

    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* Verify that MCFMC read complete pages and doesn't output all data after an EOS.
*/
quint8 StreamingThreads:: FLSH_0012(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_00 ###";
        out << "\n";

        config_CU(0,  // binning
                  1,  // test_mode
                  59,
                  10,
                  this->tmtc_channel);  // nb_bits

        quint8 bypass_flash = 0;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, bypass_flash << 3, this->tmtc_channel);
        
        // READ_EN (Read is enabled)
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        

        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1, this->tmtc_channel);
        sleep(1);
        
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after FFB", out);
        }
        
        // Format operation
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 4, this->tmtc_channel);
        sleep(20);
            
        spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
        if (spec_op_sts != 0) {
            print_log("ERROR mcfmc busy after format", out);
        }

        quint8  nb_image = 1;
        quint16 width    = 16;
        quint16 height   = 16;
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);
        sleep(2);
        cmv4000_Req_Video(width   ,   // int height,
                          height  ,   // int width,
                          0       ,   // int offset_x,
                          0       ,   // int offset_y,
                          nb_image,   // int nb_image,
                          1000       ,   // int interval
                          this->tmtc_channel);

                          
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 1, this->tmtc_channel);
        msleep(1);
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 16, this->tmtc_channel);
                          
        QByteArray video_pkt = waitPacket(this->vid_channel);
        send_TC(FLASH_CTL_ba | APB_MODE_REG, 0, this->tmtc_channel);
        (*this->rcv_data_Byte)->format_header(video_pkt);
        emit FrameReceived();

        float cmv_in_freq = 6.4;
        QString path ="./";
        QDir dir;
        path ="./DEMO/";
        if(!dir.exists(path))
        { dir.mkpath(path);}

        (*this->rcv_data_Byte)->create_txt_file(path + "picture_0.txt",  // "image_test.txt",
                                                    cmv_in_freq);

        print_log("received first sequence smaller than a page", out);
        ReceiveOperation receiveOp;
        Packet *pReceivedPacket;
        TransferOperation::TransferStatus status;
        quint32 receiveBufferLength;
        StreamItem *pStreamItem;
        quint8 *pReceiveBuffer;
        
        if (!receiveOp.CreateRxOperation(1, ReceiveOperation::RECEIVE_PACKETS))
        {
            qDebug () << "Couldn't create receive operation, exiting.";
            this->vid_channel.CloseChannel();
            return {};
        }
        this->vid_channel.SubmitTransferOperation(&receiveOp);
        print_log("before wait", out);
        status = receiveOp.WaitOnTransferOperationCompletion(5000);
        print_log("after wait", out);
        if (status == TransferOperation::TRANSFER_STATUS_COMPLETE)
        {
            qDebug () << "Error packet received while Read enable is set to 0" << status;
        }
        print_log("before GetTransferItem", out);
        pStreamItem = receiveOp.GetTransferItem(0);
        print_log("after GetTransferItem", out);
        pReceivedPacket = (Packet*)pStreamItem;
        print_log("before receiveBufferLength", out);
        /*pReceivedPacket->GetPacketData(&receiveBufferLength);
        if (receiveBufferLength != 0)
        {
            qDebug () << "Error a byte have been received" << status;
        }
        */
        print_log("after receiveBufferLength", out);

    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

/**
* ECC functionnalities of FPGA RAM memory (it shall be able to correct 1 corrupted bit and detect 2 corrupted bits).
*/
quint8 StreamingThreads:: FLSH_0013(QString file_name)
{
    QFile outputFile(file_name);
    if (outputFile.open(QIODevice::ReadWrite)) {
        QTextStream out(&outputFile);
        out << "### Start test FLSH_013 ###";
        out << "\n";

        // qDebug() << "### Reset SpaceWire";
        // send_TC(SPW_RESET_AD, 0x1, this->tmtc_channel);

        config_CU( 0, // binning,
                   1, // (*this->gui_params).test_mode,
                  60, // (*this->gui_params).ADC_gain,
                  10, // (*this->gui_params).nb_bits,
                   0, // 0,   // footer
              25000.0, // (*this->gui_params).exposure_time,
                  20, // 20,
                   4, // nb_chan,
               this->tmtc_channel);  //

        quint8  bypass_flash = 0;
        quint16 interval_ms  = 1000;

        quint8  nb_image     = 1;
        quint16 width        = 2048;
        quint16 height       = 2048;

        sleep(1);

        int dbg_pos = FLASH_DBG;
        //int dbg_pos = SDC_DBG;
        qDebug () << "### Setting DEBUG_REG to 0x" + QString::number(dbg_pos, 16);
        send_TC(DEBUG_REG_AD, dbg_pos, this->tmtc_channel );

        // Change SDRAM Buffer size
        send_TC(4, MC_SDC_ba | END_AD0, 0xFFFFFFFF, this->tmtc_channel);

        quint8 read_en = 0;

        send_TC(FLASH_CTL_ba | APB_MODE_REG, (bypass_flash << 3) | (read_en << 1), this->tmtc_channel);

        qDebug() << "---------------------------------------";
        qDebug() << "bypass flash  = " + QString::number(bypass_flash,10);
        qDebug() << "read_en       = " + QString::number(read_en,10);

        quint8 mode_reg_sts = send_TM(FLASH_CTL_ba |APB_MODE_REG , this->tmtc_channel);
        qDebug() << "Mode_Reg_Sts = 0x" + QString::number(mode_reg_sts,16);

        // Check that Bypass mode is disabled
        if( (mode_reg_sts >> 3) == 1){
            qDebug() << "Warning : Bypass FLASH mode is ENABLED!";
        }

        quint8 sd_conf_reg = send_TM(MC_SDC_ba | SD_Conf_Reg , this->tmtc_channel);
        if (sd_conf_reg != 0) {
            qDebug() << "Warning : ECC enabled inside MCSDC!";
        }

        quint8 sd_sts = send_TM(MC_SDC_ba | SD_STS , this->tmtc_channel);
        qDebug() << "SD_STS = 0x" + QString::number(sd_sts,16);
        if ((sd_sts >> 1) & 0x01 == 1) {
            qDebug() << "ERROR : SDRAM almost full!";
        }

        QByteArray end_ad_reg = send_TM(4, MC_SDC_ba | END_AD0  , this->tmtc_channel);
        quint32 end_ad_reg_int = to_int(4, end_ad_reg);
        qDebug () << "Buffer END_AD = 0x" + QString::number(end_ad_reg_int, 16);


        // Read Bad Block Array
        QByteArray BB_table;
        BB_table.clear();
        for (int i=0; i <= 3; i++) {
            BB_table.append(send_TM(128, FLASH_CTL_ba | APB_BLOCK_VALID_STS +128*i , this->tmtc_channel));
        }
        // print_data_Bytes(BB_table);

        /************************************/
        /* Corruption for ECC (RAM Blocks) */
        /***********************************/
        quint8 cor_vec = 0x40;
        //quint8 cor_vec = 0x3;
        send_TC(FLASH_CTL_ba | APB_COR_VECT2, cor_vec, this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_COR_EN, 1, this->tmtc_channel);

        quint8 Cor_Err   = send_TM(FLASH_CTL_ba |APB_RAM_CECCCNT_REG , this->tmtc_channel);
        quint8 Uncor_Err = send_TM(FLASH_CTL_ba |APB_RAM_UECCCNT_REG , this->tmtc_channel);

        qDebug() << "Initial value";
        qDebug() << "Cor_Err   = 0x" + QString::number(Cor_Err,16);
        qDebug() << "UnCor_Err = 0x" + QString::number(Uncor_Err,16);

        // Flash Initialization
        if (bypass_flash == 0){

            qDebug() << "FFB Operation";
            send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1 << 0, this->tmtc_channel);

            sleep(1);

            quint8 spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
            if (spec_op_sts != 0) {
                qDebug() << "ERROR mcfmc busy after FFB";
            }

            // Read BB table
            BB_table.clear();
            for (int i=0; i <= 3; i++) {
                BB_table.append(send_TM(128, FLASH_CTL_ba | APB_BLOCK_VALID_STS +128*i , this->tmtc_channel));
            }
            // print_data_Bytes(BB_table);


            qDebug() << "Format operation";
            send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, 1 << 2, this->tmtc_channel);

            spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
            if (spec_op_sts == (1 << 2) ) {
                qDebug() << "FORMAT operation is in progress ...";
            }
            else{
                qDebug() << "ERROR : FORMAT not started";
            }
            sleep(20);

            spec_op_sts = send_TM(FLASH_CTL_ba |APB_SPEC_CMD_STS , this->tmtc_channel);
            if (spec_op_sts != 0) {
                qDebug() << "ERROR mcfmc busy after FORMAT";
            }

            qDebug () << "End of Flash init";
        }

        sleep(1);

        // Read BB table
        BB_table.clear();
        for (int i=0; i <= 3; i++) {
            BB_table.append(send_TM(128, FLASH_CTL_ba | APB_BLOCK_VALID_STS +128*i , this->tmtc_channel));
        }
        print_data_Bytes(BB_table);


        quint8 CMV_STS = send_TM(CMV_CTL_ba | CMVCTL_STS_AD , this->tmtc_channel);
        if ((CMV_STS >> 2) & 0x1 == 1) {
            qDebug() << "ERROR bandwidth! CMV_STS = 0x"+ QString::number(CMV_STS,16);
        }

        quint8 flash_fifo_sts = send_TM(FLASH_CTL_ba |APB_FIFO_STS , this->tmtc_channel);
        if (((flash_fifo_sts >> 0) & 0x1) != 1) {
            qDebug() << "ERROR : Fifo Read NOT empty!";
            qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        }
        if ((flash_fifo_sts >> 1) != 1) {
            qDebug() << "ERROR : Fifo Write NOT empty!";
            qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        }

        end_ad_reg = send_TM(4, MC_SDC_ba | END_AD0  , this->tmtc_channel);
        end_ad_reg_int = to_int(4, end_ad_reg);
        qDebug () << "Buffer END_AD = 0x" + QString::number(end_ad_reg_int, 16);

        QByteArray NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        print_data_Bytes(NAND_STATUS);
        quint8 full = (NAND_STATUS.at(0) >> 7) & 0x1;
        quint32 nb_sequence = ((NAND_STATUS.at(0) & 0x03) << 16) | ((NAND_STATUS.at(1) & 0xFF) << 8) | (NAND_STATUS.at(2) & 0xFF);

        if (full == 1){
            qDebug() << "ERROR : Flash NAND is FULL";
        }


        qDebug () << "Video Request";
        cmv4000_Req_Video(width   ,   // int height,
                          height    ,   // int width,
                          0           ,   // int offset_x,
                          0           ,   // int offset_y,
                          nb_image,   // int nb_image,
                          interval_ms ,   // int interval
                          this->tmtc_channel);

        if(read_en == 0) {
            quint8 video_time = ceil(float(nb_image)*(float(interval_ms)/1000.0)*2.0);
            qDebug() << "Wait for " + QString::number(video_time,10) + "s";
            sleep(video_time);
        }

        int num_image = 0;
        while ((num_image < nb_image || nb_image == 255) && read_en ==1) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            qDebug () << "Packet received!!!";
            emit FrameReceived();
            qDebug () << "Size of rcv_data_Byte After = " + QString::number((*this->rcv_data_Byte)->getPacket().size(), 10);
            num_image++;
        }

        qDebug() << "After receiveBufferLength";

        qDebug() << "Preparing to Power off ...";
        send_TC(FLASH_CTL_ba | APB_POWER_CTL_REG, 0x1, this->tmtc_channel);

        CMV_STS = send_TM(CMV_CTL_ba | CMVCTL_STS_AD , this->tmtc_channel);
        if (((CMV_STS >> 2) & 0x1) == 1) {
            qDebug() << "ERROR bandwidth! CMV_STS = 0x"+ QString::number(CMV_STS,16);
        }

        if (((CMV_STS >> 3) & 0x1) == 0) {
            qDebug() << "ERROR : Video NOT Done! CMV_STS = 0x"+ QString::number(CMV_STS,16);
        }

        sleep(1);

        quint8 power_off_sts = send_TM(FLASH_CTL_ba | APB_POWER_STS, this->tmtc_channel);
        if ((power_off_sts & 0x01) != 1) {
            qDebug() << "ERROR mcfmc not ready for power off";

            qDebug() << "Prepare Power off...";
            send_TC(FLASH_CTL_ba | APB_POWER_CTL_REG, 0x1, this->tmtc_channel);

            power_off_sts = send_TM(FLASH_CTL_ba | APB_POWER_STS, this->tmtc_channel);

                if ((power_off_sts & 0x01) != 1) {
                    qDebug() << "ERROR mcfmc not ready for power off";
                }
                else {
                    qDebug () << "Ready to power off!";
                }
        }
        else {
            qDebug () << "Ready to power off!";
        }

        flash_fifo_sts = send_TM(FLASH_CTL_ba |APB_FIFO_STS , this->tmtc_channel);
        if (((flash_fifo_sts >> 0) & 0x1) != 1) {
            qDebug() << "ERROR : Fifo Read NOT empty!";
            qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        }
        if ((flash_fifo_sts >> 1) != 1) {
            qDebug() << "ERROR : Fifo Write NOT empty!";
            qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        }

        NAND_STATUS = send_TM(3, FLASH_CTL_ba | APB_NAND_ARRAY_STS0 , this->tmtc_channel);
        print_data_Bytes(NAND_STATUS);
        full = (NAND_STATUS.at(0) >> 7) & 0x1;
        nb_sequence = ((NAND_STATUS.at(0) & 0x03) << 16) | ((NAND_STATUS.at(1) & 0xFF) << 8) | (NAND_STATUS.at(2) & 0xFF);

        if (full == 1){
            qDebug() << "ERROR : Flash NAND is FULL";
        }


        sd_sts = send_TM(MC_SDC_ba | SD_STS , this->tmtc_channel);
        qDebug() << "SD_STS = 0x" + QString::number(sd_sts,16);
        if ((sd_sts >> 1) & 0x01 == 1) {
            qDebug() << "ERROR : SDRAM almost empty!";
        }

        end_ad_reg = send_TM(4, MC_SDC_ba | END_AD0  , this->tmtc_channel);
        end_ad_reg_int = to_int(4, end_ad_reg);
        qDebug () << "Buffer END_AD = 0x" + QString::number(end_ad_reg_int, 16);

        qDebug () << "Waiting for MANUAL power off...\n";

        emit Pause();

        this->tmtc_channel.CloseChannel();
        this->vid_channel.CloseChannel();

        while ((*this->gui_params).pause_clicked == false) {
            sleep(1);
            // qDebug () << "IN PAUSE";
        }
        qDebug () << "EXIT!";

        /******************************/

        // Check if channel was previously not closed
        if(this->tmtc_channel.IsOpen() == 1) {  // return 1, if the channel is open, otherwise 0.
            qDebug() << "TM/TC Channel was found opened!";
        }
        else {
            qDebug() << "TM/TC Channel " + QString::number(tmtc_channel.GetChannelNumber(), 10) + " already closed...";
        }

        SPW_interface DeviceToUse;
        DeviceToUse.find_SPW_Device();
        DeviceToUse.ResetDevice();

        DeviceToUse.config_SPW_Brick(tmtc_channelNumber, vid_channelNumber);

        qDebug() << "Opening SPW channel...";
        this->tmtc_channel = DeviceToUse.open_spw_channel(tmtc_channelNumber);
        this->vid_channel = DeviceToUse.open_spw_channel(vid_channelNumber);

        // 0x80 for DDR output mode, which is maximum bandwidth
        send_TC(TXDIVCNT_AD, 0x80, this->tmtc_channel);

        dbg_pos = FLASH_DBG;
        // dbg_pos = SDC_DBG;
        qDebug () << "### Setting DEBUG_REG to 0x" + QString::number(dbg_pos, 16);
        send_TC(DEBUG_REG_AD, dbg_pos, this->tmtc_channel );

        // Change SDRAM Buffer size
        send_TC(4, MC_SDC_ba | END_AD0, 0xFFFFFFFF, this->tmtc_channel);

        quint8 init_sts = send_TM(FLASH_CTL_ba |APB_POWER_STS , this->tmtc_channel);

        if ((init_sts >> 1) & 0x01 !=  1) {
            qDebug() << "ERROR init not completed";
        }

        quint8 erase_block0 = 1;
        send_TC(FLASH_CTL_ba | APB_SPEC_CMD_CTL, (erase_block0 << 6), this->tmtc_channel);
        sleep(1);
        quint8 spec_op_sts = send_TM(FLASH_CTL_ba | APB_SPEC_CMD_STS, this->tmtc_channel);
        if (spec_op_sts != 0) {
            qDebug() << "ERROR erase block 0 not performed";
        }

        CMV_STS = send_TM(CMV_CTL_ba | CMVCTL_STS_AD , this->tmtc_channel);
        if ((CMV_STS >> 2) & 0x1 == 1) {
            qDebug() << "ERROR bandwidth! CMV_STS = 0x"+ QString::number(CMV_STS,16);
        }

        // flash_fifo_sts = send_TM(FLASH_CTL_ba |APB_FIFO_STS , this->tmtc_channel);
        // if (((flash_fifo_sts >> 0) & 0x1) != 1) {
        //     qDebug() << "ERROR : Fifo Read NOT empty!";
        //     qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        // }
        // if ((flash_fifo_sts >> 1) != 1) {
        //     qDebug() << "ERROR : Fifo Write NOT empty!";
        //     qDebug() << "FIFO Status = 0x" + QString::number(flash_fifo_sts, 16);
        // }
        /**************************/

        // init_sts = send_TM(FLASH_CTL_ba |APB_POWER_STS , this->tmtc_channel);
        //
        // if ((init_sts >> 1) & 0x01 !=  0) {
        //     qDebug() << "ERROR init shall not be complete";
        // }

        /************************************/
        /* Corruption for ECC (RAM Blocks) */
        /***********************************/
        send_TC(FLASH_CTL_ba | APB_COR_VECT2, cor_vec, this->tmtc_channel);
        send_TC(FLASH_CTL_ba | APB_COR_EN, 1, this->tmtc_channel);


        Cor_Err   = send_TM(FLASH_CTL_ba |APB_RAM_CECCCNT_REG , this->tmtc_channel);
        Uncor_Err = send_TM(FLASH_CTL_ba |APB_RAM_UECCCNT_REG , this->tmtc_channel);

        qDebug() << "Cor_Err   = 0x" + QString::number(Cor_Err,16);
        qDebug() << "UnCor_Err = 0x" + QString::number(Uncor_Err,16);

        if (Cor_Err != 0xff) {
            qDebug() << "Cor_Err   = 0x" + QString::number(Cor_Err,16) + ", expected 0x" + QString::number(0xff,16);
        }
        if(Uncor_Err != 0xff){
            qDebug() << "Uncor_Err   = 0x" + QString::number(Uncor_Err,16) + ", expected 0x" + QString::number(0xff,16);
        }

        read_en = 1;
        send_TC(FLASH_CTL_ba | APB_MODE_REG, (bypass_flash << 3) | (read_en << 1), this->tmtc_channel);
        qDebug() << "Start to get video from Flash";
        qDebug() << "read_en = " + QString::number(read_en,10);

        num_image = 0;
        while ((num_image < nb_image || nb_image == 255) && read_en ==1) {
            QByteArray video_pkt = waitPacket(this->vid_channel);
            (*this->rcv_data_Byte)->format_header(video_pkt);
            qDebug () << "Packet received!!!";
            emit FrameReceived();
            num_image++;
        }

        Cor_Err   = send_TM(FLASH_CTL_ba |APB_RAM_CECCCNT_REG , this->tmtc_channel);
        Uncor_Err = send_TM(FLASH_CTL_ba |APB_RAM_UECCCNT_REG , this->tmtc_channel);

        qDebug() << "Cor_Err   = 0x" + QString::number(Cor_Err,16);
        qDebug() << "UnCor_Err = 0x" + QString::number(Uncor_Err,16);

        if (Cor_Err != 0xff) {
            qDebug() << "Cor_Err   = 0x" + QString::number(Cor_Err,16) + ", expected 0x" + QString::number(0xff,16);
        }
        if(Uncor_Err != 0xff){
            qDebug() << "Uncor_Err   = 0x" + QString::number(Uncor_Err,16) + ", expected 0x" + QString::number(0xff,16);
        }
        
    outputFile.close();

    }
    else {
        qDebug() << "ERROR : File not opened";
    }
    return 0;
}

