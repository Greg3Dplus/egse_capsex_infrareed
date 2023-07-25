#ifndef RIMC_REGS_H
#define RIMC_REGS_H

#include <QtGlobal>

const quint32 RIMC_BASE_AD = 0x300;

const quint32 ECC_DERR_REG0 = RIMC_BASE_AD | 0x00;
const quint32 ECC_DERR_REG1 = RIMC_BASE_AD | 0x01;
const quint32 ECC_SERR_REG0 = RIMC_BASE_AD | 0x02;
const quint32 ECC_SERR_REG1 = RIMC_BASE_AD | 0x03;

const quint32 ECC_DERR_CLR  = RIMC_BASE_AD | 0x04;
const quint32 ECC_SERR_CLR  = RIMC_BASE_AD | 0x05;

const quint32 BYPASS_REG_AD = RIMC_BASE_AD | 0x06;
  const quint32 BYPASS_gm   = 0x01;
  const quint32 BYPASS_gp   =    0;
  const quint32 MODE_gm     = 0x06;
  const quint32 MODE_gp     =    1;
  const quint32 DLL_gm      = 0x08;
  const quint32 DLL_gp      =    3;

const quint32 INIT_PAD_REG  = RIMC_BASE_AD | 0x07;
const quint32 INIT_PAD_STS  = RIMC_BASE_AD | 0x08;
const quint32 INIT_PAD_CTL  = RIMC_BASE_AD | 0x09;

const quint32 SCRUB_REG0    = RIMC_BASE_AD | 0x0A;
const quint32 SCRUB_REG1    = RIMC_BASE_AD | 0x0B;
const quint32 SCRUBEN_REG   = RIMC_BASE_AD | 0x0C;
const quint32 SCRUB_CTL     = RIMC_BASE_AD | 0x0D;

const quint32 COREN_REG     = RIMC_BASE_AD | 0x0E;
const quint32 CORBEAT_REG   = RIMC_BASE_AD | 0x0F;
const quint32 CORVEC_REG0   = RIMC_BASE_AD | 0x10;
const quint32 CORVEC_REG1   = RIMC_BASE_AD | 0x11;
const quint32 CORVEC_REG2   = RIMC_BASE_AD | 0x12;
const quint32 CORVEC_REG3   = RIMC_BASE_AD | 0x13;
const quint32 CORVEC_REG4   = RIMC_BASE_AD | 0x14;
const quint32 CORVEC_REG5   = RIMC_BASE_AD | 0x15;
const quint32 CORVEC_REG6   = RIMC_BASE_AD | 0x16;
const quint32 CORVEC_REG7   = RIMC_BASE_AD | 0x17;
const quint32 CORVEC_REG8   = RIMC_BASE_AD | 0x18;
const quint32 CORVEC_REG9   = RIMC_BASE_AD | 0x19;
const quint32 CORVEC_REG10  = RIMC_BASE_AD | 0x1A;
const quint32 CORVEC_REG11  = RIMC_BASE_AD | 0x1B;

const quint32 SCRUBAD_STS0  = RIMC_BASE_AD | 0x1C;
const quint32 SCRUBAD_STS1  = RIMC_BASE_AD | 0x1D;
const quint32 SCRUBAD_STS2  = RIMC_BASE_AD | 0x1E;
const quint32 SCRUBAD_STS3  = RIMC_BASE_AD | 0x1F;

const quint32 RIMC_VERSION_STS   = RIMC_BASE_AD | 0x20;

#endif // RIMC_REGS_H
