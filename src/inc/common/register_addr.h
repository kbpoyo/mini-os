#ifndef REGISTER_ADDR_H
#define REGISTER_ADDR_H

#include "os_config.h"

// 看门狗相关寄存器
//  WATCH DOG TIMER
#define _rWTCON 0x53000000
#define _rWTDAT 0x53000004
#define _rWTCNT 0x53000008
#define rWTCON (*(volatile unsigned *)_rWTCON)  // Watch-dog timer mode
#define rWTDAT (*(volatile unsigned *)_rWTDAT)  // Watch-dog timer data
#define rWTCNT (*(volatile unsigned *)_rWTCNT)  // Eatch-dog timer count

// 时钟和电源相关寄存器
//  CLOCK & POWER MANAGEMENT
#define _rLOCKTIME 0x4c000000
#define _rMPLLCON 0x4c000004
#define _rUPLLCON 0x4c000008
#define _rCLKCON 0x4c00000c
#define _rCLKSLOW 0x4c000010
#define _rCLKDIVN 0x4c000014
#define _rCAMDIVN 0x4c000018

#define rLOCKTIME (*(volatile unsigned *)_rLOCKTIME)  // PLL lock time counter
#define rMPLLCON (*(volatile unsigned *)_rMPLLCON)    // MPLL Control
#define rUPLLCON (*(volatile unsigned *)_rUPLLCON)    // UPLL Control
#define rCLKCON (*(volatile unsigned *)_rCLKCON)      // Clock generator control
#define rCLKSLOW (*(volatile unsigned *)_rCLKSLOW)    // Slow clock control
#define rCLKDIVN (*(volatile unsigned *)_rCLKDIVN)    // Clock divider control
#define rCAMDIVN \
  (*(volatile unsigned *)_rCAMDIVN)  // USB, CAM Clock divider control

#define rLOCKTIME_INIT 0xffffffff  // 大于300us即可
// 设置时钟分频寄存器
#define rCLKDIVN_INIT \
  ((0x2 << 1) | (1 << 0))  // FCLK:HCLK:PCLK=4:1:0.5 ,需要设置进入异步总线模式
// 设置摄像头时钟分频寄存器
#define rCAMDIVN_INIT 0x0

// UPLL=48mhz
#define U_MDIV 0x38
#define U_PDIV 0x2
#define U_SDIV 0x2
// FCLK = MPLL = 400Mhz(数据手册中没有准确的400mhz，浏览器搜索获得)
#define M_MDIV 0x5c
#define M_PDIV 0x1
#define M_SDIV 0x1

// 设置时钟控制寄存器
#define rCLKCON_INIT 0xfffff0

// 中断相关寄存器
//  INTERRUPT
#define _rSRCPND 0x4a000000
#define _rINTMOD 0x4a000004
#define _rINTMSK 0x4a000008
#define _rPRIORITY 0x4a00000c
#define _rINTPND 0x4a000010
#define _rINTOFFSET 0x4a000014
#define _rSUBSRCPND 0x4a000018
#define _rINTSUBMSK 0x4a00001c

#define rSRCPND (*(volatile unsigned *)_rSRCPND)  // 中断源未决寄存器
#define rINTMOD (*(volatile unsigned *)_rINTMOD)  // Interrupt mode control
#define rINTMSK (*(volatile unsigned *)_rINTMSK)  // Interrupt mask control
#define rPRIORITY (*(volatile unsigned *)_rPRIORITY)  // IRQ priority control
#define rINTPND (*(volatile unsigned *)_rINTPND)  // 中断未决寄存器，仅有1位置1
#define rINTOFFSET        \
  (*(volatile unsigned *) \
       _rINTOFFSET)  // 中断偏移量寄存器，发生中断后记录当前发生的中断向量号
#define rSUBSRCPND (*(volatile unsigned *)_rSUBSRCPND)  // Sub source pending
#define rINTSUBMSK (*(volatile unsigned *)_rINTSUBMSK)  // Interrupt sub mask

// SDRAM相关寄存器
//  Memory control
#define _rBWSCON 0x48000000
#define _rBANKCON0 0x48000004
#define _rREFRESH 0x48000024
#define _rBANKSIZE 0x48000028
#define _rMRSRB6 0x4800002c
#define _rMRSRB7 0x48000030

#define rBWSCON (*(volatile unsigned *)_rBWSCON)  // Bus width & wait 状态寄存器
#define rBANKCON0 (*(volatile unsigned *)_rBANKCON0)  // Boot ROM control
#define rBANKCON1 (*(volatile unsigned *)0x48000008)  // BANK1 control
#define rBANKCON2 (*(volatile unsigned *)0x4800000c)  // BANK2 cControl
#define rBANKCON3 (*(volatile unsigned *)0x48000010)  // BANK3 control
#define rBANKCON4 (*(volatile unsigned *)0x48000014)  // BANK4 control
#define rBANKCON5 (*(volatile unsigned *)0x48000018)  // BANK5 control
#define rBANKCON6 (*(volatile unsigned *)0x4800001c)  // BANK6 control
#define rBANKCON7 (*(volatile unsigned *)0x48000020)  // BANK7 control
#define rREFRESH (*(volatile unsigned *)0x48000024)   // DRAM/SDRAM refresh
#define rBANKSIZE (*(volatile unsigned *)0x48000028)  // Flexible Bank Size
#define rMRSRB6 \
  (*(volatile unsigned *)0x4800002c)  // Mode register set for SDRAM
#define rMRSRB7 \
  (*(volatile unsigned *)0x48000030)  // Mode register set for SDRAM

// bank1~bank7每4位控制一个bank 4位由高到低分别是ST,WS,DW(占两位)
// ST（启动/禁止SDRAM的数据掩码引脚。对于SDRAM，此位置0；对于SRAM，此位置1）
// WS（是否使用存储器的WAIT信号，通常置0为不使用）
// DW（两位，设置位宽。此板子的SDRAM是32位，故将DW6设为10）
#define rBWSCON_INIT 0x22000000

// bank控制寄存器相关位设置
#define rBANKCON_0_5_INIT (0x0700)
#define rBANKCON6_INIT \
  ((0x3 << 15) | (0x1 << 2) | (0x1 << 0))  // banck6,7为SDRAM且有9-bit列地址位数
#define rBANKCON7_INIT ((0x3 << 15) | (0x1 << 2) | (0x1 << 0))
// 刷新控制寄存器设置
#define rREFRESH_INIT                        \
  ((0x1 << 23) | (0x1 << 20) | (0x2 << 18) | \
   1268)  //((1 << 11) + 1 - SDRAM_REFRESH_TIME * OS_HCLK / 1e6)

// BANKSIZE寄存器设置
#define rBANKSIZE_INIT ((0x1 << 7) | (0x1 << 5) | (0x1 << 4) | (0x1 << 0))

// SDRAM模式寄存器设置
#define rMRSRB_INIT (0x3 << 4)

// NANDFLASH相关寄存器
#define rNFCONF (*(volatile unsigned *)0x4E000000)  // NAND Flash configuration
#define rNFCONT (*(volatile unsigned *)0x4E000004)  // NAND Flash control
#define rNFCMD (*(volatile unsigned *)0x4E000008)   // NAND Flash command
#define rNFADDR (*(volatile unsigned *)0x4E00000C)  // NAND Flash address
#define rNFDATA (*(volatile unsigned *)0x4E000010)  // NAND Flash data
#define rNFDATA8 (*(volatile unsigned char *)0x4E000010)  // NAND Flash data

#define rNFMECCD0 \
  (*(volatile unsigned *)0x4E000014)  // NAND Flash ECC for Main Area
#define rNFMECCD1 (*(volatile unsigned *)0x4E000018)
#define rNFSECCD \
  (*(volatile unsigned *)0x4E00001C)  // NAND Flash ECC for Spare Area
#define rNFSTAT \
  (*(volatile unsigned char *)0x4E000020)  // NAND Flash operation status
#define rNFESTAT0 (*(volatile unsigned *)0x4E000024)
#define rNFESTAT1 (*(volatile unsigned *)0x4E000028)
#define rNFMECC0 (*(volatile unsigned *)0x4E00002C)
#define rNFMECC1 (*(volatile unsigned *)0x4E000030)
#define rNFSECC (*(volatile unsigned *)0x4E000034)
#define rNFSBLK \
  (*(volatile unsigned *)0x4E000038)  // NAND Flash Start block address
#define rNFEBLK \
  (*(volatile unsigned *)0x4E00003C)  // NAND Flash End block address

// UART
#define rULCON0 (*(volatile unsigned *)0x50000000)    // UART 0 Line control
#define rUCON0 (*(volatile unsigned *)0x50000004)     // UART 0 Control
#define rUFCON0 (*(volatile unsigned *)0x50000008)    // UART 0 FIFO control
#define rUMCON0 (*(volatile unsigned *)0x5000000c)    // UART 0 Modem control
#define rUTRSTAT0 (*(volatile unsigned *)0x50000010)  // UART 0 Tx/Rx status
#define rUERSTAT0 (*(volatile unsigned *)0x50000014)  // UART 0 Rx error status
#define rUFSTAT0 (*(volatile unsigned *)0x50000018)   // UART 0 FIFO status
#define rUMSTAT0 (*(volatile unsigned *)0x5000001c)   // UART 0 Modem status
#define rUBRDIV0 (*(volatile unsigned *)0x50000028)  // UART 0 Baud rate divisor

#define rULCON1 (*(volatile unsigned *)0x50004000)    // UART 1 Line control
#define rUCON1 (*(volatile unsigned *)0x50004004)     // UART 1 Control
#define rUFCON1 (*(volatile unsigned *)0x50004008)    // UART 1 FIFO control
#define rUMCON1 (*(volatile unsigned *)0x5000400c)    // UART 1 Modem control
#define rUTRSTAT1 (*(volatile unsigned *)0x50004010)  // UART 1 Tx/Rx status
#define rUERSTAT1 (*(volatile unsigned *)0x50004014)  // UART 1 Rx error status
#define rUFSTAT1 (*(volatile unsigned *)0x50004018)   // UART 1 FIFO status
#define rUMSTAT1 (*(volatile unsigned *)0x5000401c)   // UART 1 Modem status
#define rUBRDIV1 (*(volatile unsigned *)0x50004028)  // UART 1 Baud rate divisor

#define rULCON2 (*(volatile unsigned *)0x50008000)    // UART 2 Line control
#define rUCON2 (*(volatile unsigned *)0x50008004)     // UART 2 Control
#define rUFCON2 (*(volatile unsigned *)0x50008008)    // UART 2 FIFO control
#define rUMCON2 (*(volatile unsigned *)0x5000800c)    // UART 2 Modem control
#define rUTRSTAT2 (*(volatile unsigned *)0x50008010)  // UART 2 Tx/Rx status
#define rUERSTAT2 (*(volatile unsigned *)0x50008014)  // UART 2 Rx error status
#define rUFSTAT2 (*(volatile unsigned *)0x50008018)   // UART 2 FIFO status
#define rUMSTAT2 (*(volatile unsigned *)0x5000801c)   // UART 2 Modem status
#define rUBRDIV2 (*(volatile unsigned *)0x50008028)  // UART 2 Baud rate divisor

#ifdef __BIG_ENDIAN
#define rUTXH0 \
  (*(volatile unsigned char *)0x50000023)  // UART 0 Transmission Hold
#define rURXH0 (*(volatile unsigned char *)0x50000027)  // UART 0 Receive buffer
#define rUTXH1 \
  (*(volatile unsigned char *)0x50004023)  // UART 1 Transmission Hold
#define rURXH1 (*(volatile unsigned char *)0x50004027)  // UART 1 Receive buffer
#define rUTXH2 \
  (*(volatile unsigned char *)0x50008023)  // UART 2 Transmission Hold
#define rURXH2 (*(volatile unsigned char *)0x50008027)  // UART 2 Receive buffer

#define WrUTXH0(ch) \
  (*(volatile unsigned char *)0x50000023) = (unsigned char)(ch)
#define RdURXH0() (*(volatile unsigned char *)0x50000027)
#define WrUTXH1(ch) \
  (*(volatile unsigned char *)0x50004023) = (unsigned char)(ch)
#define RdURXH1() (*(volatile unsigned char *)0x50004027)
#define WrUTXH2(ch) \
  (*(volatile unsigned char *)0x50008023) = (unsigned char)(ch)
#define RdURXH2() (*(volatile unsigned char *)0x50008027)

#define UTXH0 (0x50000020 + 3)  // Byte_access address by DMA
#define URXH0 (0x50000024 + 3)
#define UTXH1 (0x50004020 + 3)
#define URXH1 (0x50004024 + 3)
#define UTXH2 (0x50008020 + 3)
#define URXH2 (0x50008024 + 3)

#else  // Little Endian
#define rUTXH0 \
  (*(volatile unsigned char *)0x50000020)  // UART 0 Transmission Hold
#define rURXH0 (*(volatile unsigned char *)0x50000024)  // UART 0 Receive buffer
#define rUTXH1 \
  (*(volatile unsigned char *)0x50004020)  // UART 1 Transmission Hold
#define rURXH1 (*(volatile unsigned char *)0x50004024)  // UART 1 Receive buffer
#define rUTXH2 \
  (*(volatile unsigned char *)0x50008020)  // UART 2 Transmission Hold
#define rURXH2 (*(volatile unsigned char *)0x50008024)  // UART 2 Receive buffer

#define WrUTXH0(ch) \
  (*(volatile unsigned char *)0x50000020) = (unsigned char)(ch)
#define RdURXH0() (*(volatile unsigned char *)0x50000024)
#define WrUTXH1(ch) \
  (*(volatile unsigned char *)0x50004020) = (unsigned char)(ch)
#define RdURXH1() (*(volatile unsigned char *)0x50004024)
#define WrUTXH2(ch) \
  (*(volatile unsigned char *)0x50008020) = (unsigned char)(ch)
#define RdURXH2() (*(volatile unsigned char *)0x50008024)

#define UTXH0 (0x50000020)  // Byte_access address by DMA
#define URXH0 (0x50000024)
#define UTXH1 (0x50004020)
#define URXH1 (0x50004024)
#define UTXH2 (0x50008020)
#define URXH2 (0x50008024)

#endif

// I/O PORT
#define rGPACON (*(volatile unsigned *)0x56000000)  // Port A control
#define rGPADAT (*(volatile unsigned *)0x56000004)  // Port A data

#define rGPBCON (*(volatile unsigned *)0x56000010)  // Port B control
#define rGPBDAT (*(volatile unsigned *)0x56000014)  // Port B data
#define rGPBUP (*(volatile unsigned *)0x56000018)   // Pull-up control B

#define rGPCCON (*(volatile unsigned *)0x56000020)  // Port C control
#define rGPCDAT (*(volatile unsigned *)0x56000024)  // Port C data
#define rGPCUP (*(volatile unsigned *)0x56000028)   // Pull-up control C

#define rGPDCON (*(volatile unsigned *)0x56000030)  // Port D control
#define rGPDDAT (*(volatile unsigned *)0x56000034)  // Port D data
#define rGPDUP (*(volatile unsigned *)0x56000038)   // Pull-up control D

#define rGPECON (*(volatile unsigned *)0x56000040)  // Port E control
#define rGPEDAT (*(volatile unsigned *)0x56000044)  // Port E data
#define rGPEUP (*(volatile unsigned *)0x56000048)   // Pull-up control E

#define rGPFCON (*(volatile unsigned *)0x56000050)  // Port F control
#define rGPFDAT (*(volatile unsigned *)0x56000054)  // Port F data
#define rGPFUP (*(volatile unsigned *)0x56000058)   // Pull-up control F

#define rGPGCON (*(volatile unsigned *)0x56000060)  // Port G control
#define rGPGDAT (*(volatile unsigned *)0x56000064)  // Port G data
#define rGPGUP (*(volatile unsigned *)0x56000068)   // Pull-up control G

#define rGPHCON (*(volatile unsigned *)0x56000070)  // Port H control
#define rGPHDAT (*(volatile unsigned *)0x56000074)  // Port H data
#define rGPHUP (*(volatile unsigned *)0x56000078)   // Pull-up control H

#define rGPJCON (*(volatile unsigned *)0x560000d0)  // Port J control
#define rGPJDAT (*(volatile unsigned *)0x560000d4)  // Port J data
#define rGPJUP (*(volatile unsigned *)0x560000d8)   // Pull-up control J

#define rMISCCR (*(volatile unsigned *)0x56000080)   // Miscellaneous control
#define rDCLKCON (*(volatile unsigned *)0x56000084)  // DCLK0/1 control
#define rEXTINT0 \
  (*(volatile unsigned *)0x56000088)  // External interrupt control register 0
#define rEXTINT1 \
  (*(volatile unsigned *)0x5600008c)  // External interrupt control register 1
#define rEXTINT2 \
  (*(volatile unsigned *)0x56000090)  // External interrupt control register 2
#define rEINTFLT0 (*(volatile unsigned *)0x56000094)  // Reserved
#define rEINTFLT1 (*(volatile unsigned *)0x56000098)  // Reserved
#define rEINTFLT2      \
  (*(volatile unsigned \
         *)0x5600009c)  // External interrupt filter control register 2
#define rEINTFLT3      \
  (*(volatile unsigned \
         *)0x560000a0)  // External interrupt filter control register 3
#define rEINTMASK (*(volatile unsigned *)0x560000a4)  // External interrupt mask
#define rEINTPEND \
  (*(volatile unsigned *)0x560000a8)  // External interrupt pending
#define rGSTATUS0 (*(volatile unsigned *)0x560000ac)  // External pin status
#define rGSTATUS1 (*(volatile unsigned *)0x560000b0)  // Chip ID(0x32440000)
#define rGSTATUS2 (*(volatile unsigned *)0x560000b4)  // Reset type
#define rGSTATUS3      \
  (*(volatile unsigned \
         *)0x560000b8)  // Saved data0(32-bit) before entering POWER_OFF mode
#define rGSTATUS4      \
  (*(volatile unsigned \
         *)0x560000bc)  // Saved data0(32-bit) before entering POWER_OFF mode

// 定时器相关寄存器
//  PWM TIMER
#define rTCFG0 (*(volatile unsigned *)0x51000000)   // Timer 0 configuration
#define rTCFG1 (*(volatile unsigned *)0x51000004)   // Timer 1 configuration
#define rTCON (*(volatile unsigned *)0x51000008)    // Timer control
#define rTCNTB0 (*(volatile unsigned *)0x5100000c)  // Timer count buffer 0
#define rTCMPB0 (*(volatile unsigned *)0x51000010)  // Timer compare buffer 0
#define rTCNTO0 (*(volatile unsigned *)0x51000014)  // Timer count observation 0
#define rTCNTB1 (*(volatile unsigned *)0x51000018)  // Timer count buffer 1
#define rTCMPB1 (*(volatile unsigned *)0x5100001c)  // Timer compare buffer 1
#define rTCNTO1 (*(volatile unsigned *)0x51000020)  // Timer count observation 1
#define rTCNTB2 (*(volatile unsigned *)0x51000024)  // Timer count buffer 2
#define rTCMPB2 (*(volatile unsigned *)0x51000028)  // Timer compare buffer 2
#define rTCNTO2 (*(volatile unsigned *)0x5100002c)  // Timer count observation 2
#define rTCNTB3 (*(volatile unsigned *)0x51000030)  // Timer count buffer 3
#define rTCMPB3 (*(volatile unsigned *)0x51000034)  // Timer compare buffer 3
#define rTCNTO3 (*(volatile unsigned *)0x51000038)  // Timer count observation 3
#define rTCNTB4 (*(volatile unsigned *)0x5100003c)  // Timer count buffer 4
#define rTCNTO4 (*(volatile unsigned *)0x51000040)  // Timer count observation 4

#endif