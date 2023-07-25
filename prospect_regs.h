
#ifndef PROSPECT_REGS_H
#define PROSPECT_REGS_H

#include <QtGlobal>

// For variable cam_hw_type
const quint8 CAM_HW_3DCM734    = 0;
const quint8 CAM_HW_3DCM739    = 1;
const quint8 CAM_HW_CO2M       = 2;
const quint8 CAM_HW_CINEMA     = 3;
const quint8 CAM_HW_CASPEX_IR  = 4;

// For variable cam_fw_type
const quint8 CAM_FW_3DIPCC0746 = 0;
const quint8 CAM_FW_3DIPCC0802 = 1;
const quint8 CAM_FW_CO2M       = 2;
const quint8 CAM_FW_CINEMA     = 3;
const quint8 CAM_FW_CASPEX_IR  = 4;

// FOR IMX 54
const quint8 GTWAIT  = 6;
const quint8 VB1[2] = {38, 3+GTWAIT};
const quint8 frameInfo[2] = {16,1+8+12+2}; //=4+1+6+4+1  cf page 53 datasheet imx

////////////////////////////////////////
// Memory Area
////////////////////////////////////////
const quint32 AHBS_APBCTRL_ba      = 0x00000000;
const quint32 AHBSLV_SPI_ba_3DIPCC = 0x00000800;
const quint32 AHBSLV_SPI_ba_CO2M   = 0x00001000;
const quint32 AHBSLV_SPI_ba_CINEMA = 0x00010000;
const quint32 AHBSLV_DDR2_ba       = 0x10000000;

////////////////////////////////////////
// APB registers
////////////////////////////////////////

const quint32 APB_REG_ba             = AHBS_APBCTRL_ba | 0x200;

const quint32 DEBUG_REG_AD           = APB_REG_ba | 0x00;
const quint32 VERSION_STS_AD         = APB_REG_ba | 0x01;
const quint32 CMVCLK_DIV_REG_AD      = APB_REG_ba | 0x02;

const quint32 SPW_CTL_ba             = AHBS_APBCTRL_ba | 0x380;

const quint32 CL_CTL_ba              = AHBS_APBCTRL_ba | 0x100;

const quint32 TXDIVCNT_AD            = SPW_CTL_ba | 0x00;
const quint32 BYTESEND_CTL_AD        = SPW_CTL_ba | 0x01;
const quint32 SPW_ERR_STS_AD         = SPW_CTL_ba | 0x02;
const quint32 SPW_ERR_CLR_AD         = SPW_CTL_ba | 0x03;
const quint32 SPW_RESET_AD           = SPW_CTL_ba | 0x04;
const quint32 NBBYTES_AD             = SPW_CTL_ba | 0x05;

const quint32 SENS_CTL_ba             = AHBS_APBCTRL_ba | 0x000;

const quint32 MC_DDC_ba              = AHBS_APBCTRL_ba | 0x100;
const quint32 MC_CL_ba               = AHBS_APBCTRL_ba | 0x180;

const quint32 FLASH_CTL_ba           = AHBS_APBCTRL_ba | 0x400;

/*////////////////////////////////////////
// DEBUG POSITION
////////////////////////////////////////*/
const quint8 USER_DBG     = 0x0;
const quint8 AHB_DBG      = 0x1;
const quint8 CMV4000_DBG  = 0x2;
const quint8 MICTOR_DBG   = 0x3;
const quint8 FLASH_DBG    = 0x4;
const quint8 SDC_DBG      = 0x5;

#endif /* PROSPECT_REGS_H */

