#ifndef MCFMC_REGISTERS_H
#define MCFMC_REGISTERS_H

const unsigned MCFMC_VERSION                      = 27;

const unsigned APB_SPEC_CMD_CTL                   = 0x00000;
const unsigned APB_SPEC_CMD_STS                   = 0x00001;
const unsigned APB_ECC_CNT_REG0                   = 0x00002;
const unsigned APB_ECC_CNT_REG1                   = 0x00003;
const unsigned APB_ECC_ERR_STS                    = 0x00004;
const unsigned APB_ECC_ERR_CTL                    = 0x00005;
const unsigned APB_BLOCKERR_STS0                  = 0x00006;
const unsigned APB_BLOCKERR_STS1                  = 0x00007;
const unsigned APB_BLOCKERR_CTL                   = 0x00008;
const unsigned APB_MODE_REG                       = 0x00009;
const unsigned APB_POWER_CTL_REG                  = 0x0000A;
const unsigned APB_POWER_STS                      = 0x0000B;
const unsigned APB_FIFO_STS                       = 0x0000C;
const unsigned APB_NAND_ARRAY_STS0                = 0x0000D;
const unsigned APB_NAND_ARRAY_STS1                = 0x0000E;
const unsigned APB_NAND_ARRAY_STS2                = 0x0000F;
const unsigned APB_COR_VECT0                      = 0x00010;
const unsigned APB_COR_VECT1                      = 0x00011;
const unsigned APB_COR_VECT2                      = 0x00012;
const unsigned APB_COR_EN                         = 0x00013;
const unsigned APB_POINTER_WRITE0_STS             = 0x00014;
const unsigned APB_POINTER_WRITE1_STS             = 0x00015;
const unsigned APB_POINTER_WRITE2_STS             = 0x00016;
const unsigned APB_POINTER_READ_CURRENT0_STS      = 0x00017;
const unsigned APB_POINTER_READ_CURRENT1_STS      = 0x00018;
const unsigned APB_POINTER_READ_CURRENT2_STS      = 0x00019;
const unsigned APB_POINTER_READ_LAST_STSEQ0_STS   = 0x0001A;
const unsigned APB_POINTER_READ_LAST_STSEQ1_STS   = 0x0001B;
const unsigned APB_POINTER_READ_LAST_STSEQ2_STS   = 0x0001C;
const unsigned APB_POINTER_ERASE0_STS             = 0x0001D;
const unsigned APB_POINTER_ERASE1_STS             = 0x0001E;
const unsigned APB_VERSION_STS                    = 0x0001F;
const unsigned APB_ADDR_FAIL0_REG                 = 0x00020;
const unsigned APB_ADDR_FAIL1_REG                 = 0x00021;
const unsigned APB_ADDR_FAIL2_REG                 = 0x00022;
const unsigned APB_OP_FAIL_REG                    = 0x00023;
const unsigned APB_TIMEOUT_CNT0_REG               = 0x00024;
const unsigned APB_TIMEOUT_CNT1_REG               = 0x00025;
const unsigned APB_TIMEOUT_CNT2_REG               = 0x00026;
const unsigned APB_RAM_CECCCNT_REG                = 0x00027;
const unsigned APB_RAM_UECCCNT_REG                = 0x00028;
const unsigned APB_BLOCK_VALID_STS                = 0x00200;

const unsigned FFB_FAIL                           = 0x00001;
const unsigned READ_OP_TIMEOUT                    = 0x00002;
const unsigned WRITE_OP_FAIL                      = 0x00003;
const unsigned WRITE_OP_TIMEOUT                   = 0x00004;
const unsigned ERASE_OP_FAIL                      = 0x00005;
const unsigned ERASE_OP_TIMEOUT                   = 0x00006;
const unsigned READ_CORRUPT                       = 0x00007;


const unsigned MCFMC_READ_LAST_REQ                = 0x20;
const unsigned MCFMC_READ_CONFIRM_MODE            = 0x03;
const unsigned MCFMC_READ_NOMINAL_MODE            = 0x02;
const unsigned MCFMC_BYPASS_MODE                  = 0x08;
#endif // MCFMC_REGISTERS_H
