
#ifndef CMV_REGISTERS_H
#define CMV_REGISTERS_H

////////////////////////////////////////
// APB registers
////////////////////////////////////////       

const unsigned MCCMV_VERSION            = 47;

const unsigned VERSION_STS              = 0x00;
const unsigned CMVCTL_CONF_AD           = 0x01;
                                      
const unsigned NB_IMAGE_AD              = 0x02;
const unsigned INTER_IMAGE_AD0          = 0x03;
const unsigned INTER_IMAGE_AD1          = 0x04;
const unsigned INTER_IMAGE_AD2          = 0x05;
const unsigned INTER_IMAGE_AD3          = 0x06;
                                   
const unsigned CMVCTL_CTL_AD            = 0x07;
const unsigned CMVCTL_CTL_TRAINING_gm   = 0x02;
const unsigned CMVCTL_CTL_VIDEO_REQ_gm  = 0x01;
const unsigned CMVCTL_CTL_ABORT_REQ_gm  = 0x04;
const unsigned CMVCTL_CTL_CLR_ERROR_gm  = 0x08;
const unsigned CMVCTL_CTL_RST_SERDES_gm = 0x10;
                                     
const unsigned FRAME_TAG_AD0            = 0x08;
const unsigned FRAME_TAG_AD1            = 0x09;
const unsigned FRAME_TAG_AD2            = 0x0A;
const unsigned FRAME_TAG_AD3            = 0x0B;
const unsigned FRAME_TAG_AD4            = 0x0C;
const unsigned FRAME_TAG_AD5            = 0x0D;
const unsigned FRAME_TAG_AD6            = 0x0E;
const unsigned FRAME_TAG_AD7            = 0x0F;
                                
const unsigned CMVCTL_SWITCH2V1_AD0     = 0x10;
const unsigned CMVCTL_SWITCH2V1_AD1     = 0x11;
const unsigned CMVCTL_SWITCH3V3_AD0     = 0x12;
const unsigned CMVCTL_SWITCH3V3_AD1     = 0x13;
                                      
const unsigned CMVCTL_LATCHUP_CNT_REG   = 0x14;
                                
const unsigned CMVCTL_PIXSTART_AD0      = 0x15;
const unsigned CMVCTL_PIXSTART_AD1      = 0x16;
const unsigned CMVCTL_PIXLAST_AD0       = 0x17;
const unsigned CMVCTL_PIXLAST_AD1       = 0x18;
                                      
const unsigned CMVCTL_ROWSTART_AD0      = 0x19;
const unsigned CMVCTL_ROWSTART_AD1      = 0x1A;
const unsigned CMVCTL_ROWWIDTH_AD0      = 0x1B;
const unsigned CMVCTL_ROWWIDTH_AD1      = 0x1C;
                                  
const unsigned CMVCTL_STS_AD            = 0x1D;
const unsigned CMVCTL_STS_VID_DONE_gm   = 0x08;
const unsigned CMVCTL_STS_AHBERR_gm     = 0x04;
const unsigned CMVCTL_STS_TRDONE_gm     = 0x02;
const unsigned CMVCTL_STS_FRDONE_gm     = 0x01;

const unsigned TRIG_EXP_TIME_AD0        = 0x1E;
const unsigned TRIG_EXP_TIME_AD1        = 0x1F;
const unsigned TRIG_EXP_TIME_AD2        = 0x20;
const unsigned TRIG_EXP_TIME_AD3        = 0x21;
                                  
const unsigned FREQ_DIV                 = 0x22;

const unsigned EXP_TIME_AD0             = 0x23;
const unsigned EXP_TIME_AD1             = 0x24;
const unsigned EXP_TIME_AD2             = 0x25;

const unsigned ECC_COR_REG_AD           = 0x26;
const unsigned ECC_UNCOR_REG_AD         = 0x27;
const unsigned CORVEC_REG_AD0           = 0x28;
const unsigned CORVEC_REG_AD1           = 0x29;
const unsigned CORVEC_REG_AD2           = 0x2A;
                          
const unsigned CALIB_STS_AD0            = 0x2B;
const unsigned CALIB_STS_AD1            = 0x2C;

const unsigned CMVCTL_BINNING_X_AD      = 0x2E;
const unsigned CMVCTL_BINNING_Y_AD      = 0x2F;


const unsigned SENSCTL_TRAINING_PATTERN_AD0     = 0x30;
const unsigned SENSCTL_TRAINING_PATTERN_AD1     = 0x31;

const unsigned SENSCTL_FRAME_RATE_AD            = 0x32;
const unsigned SENSCTL_NUM_CHAN_AD              = 0x33;

#endif /* CMV_REGISTERS_H */
