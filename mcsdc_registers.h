#ifndef MCSDC_REGISTERS_H
#define MCSDC_REGISTERS_H

const unsigned MCSDC_VERSION      = 31;

const unsigned SD_Version_STS     = 0x00;
const unsigned SD_Conf_Reg        = 0x01;
const unsigned SD_CTL             = 0x02;
const unsigned SD_STS             = 0x03;
const unsigned END_AD0            = 0x04;
const unsigned END_AD1            = 0x05;
const unsigned END_AD2            = 0x06;
const unsigned END_AD3            = 0x07;
const unsigned ERR_AD0            = 0x08;
const unsigned ERR_AD1            = 0x09;
const unsigned ERR_AD2            = 0x0A;
const unsigned ERR_AD3            = 0x0B;
const unsigned ECC_CNT0           = 0x0C;
const unsigned ECC_CNT1           = 0x0D;
const unsigned CURR_READ0         = 0x0E;
const unsigned CURR_READ1         = 0x0F;
const unsigned CURR_READ2         = 0x10;
const unsigned CURR_READ3         = 0x11;
const unsigned CURR_WRITE0        = 0x12;
const unsigned CURR_WRITE1        = 0x13;
const unsigned CURR_WRITE2        = 0x14;
const unsigned CURR_WRITE3        = 0x15;
const unsigned COR_VEC_AD0        = 0x16;
const unsigned COR_VEC_AD1        = 0x17;

const unsigned ECC_EN               = 0;
const unsigned REF_PER              = 1;
const unsigned SCRUB_EN             = 2;

const char     mcsdc_regs_value[24] = {  MCSDC_VERSION, //SD_Version_STS
                                       0x00, // SD_Conf_Reg
                                       0x00, // SD_CTL
                                       0x04, // SD_STS
                                       0x00, // END_AD0
                                       0x00, // END_AD1
                                       0x00, // END_AD2
                                       0x04, // END_AD3
                                       0x00, // ERR_AD0
                                       0x00, // ERR_AD1
                                       0x00, // ERR_AD2
                                       0x00, // ERR_AD3
                                       0x00, // ECC_CNT0
                                       0x00, // ECC_CNT1
                                       0x00, // CURR_READ0
                                       0x00, // CURR_READ1
                                       0x00, // CURR_READ2
                                       0x00, // CURR_READ3
                                       0x00, // CURR_WRITE0
                                       0x00, // CURR_WRITE1
                                       0x00, // CURR_WRITE2
                                       0x00, // CURR_WRITE3
                                       0x00, // COR_VEC_AD0
                                       0x00  // COR_VEC_AD1
                                       };

#endif // MCSDC_REGISTERS_H
