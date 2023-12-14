#ifndef REGISTER_ADDR_H
#define REGISTER_ADDR_H

#include "os_config.h"

//看门狗相关寄存器
// WATCH DOG TIMER
#define _rWTCON   0x53000000
#define _rWTDAT   0x53000004
#define _rWTCNT   0x53000008
#define rWTCON   (*(volatile unsigned *)_rWTCON)	//Watch-dog timer mode
#define rWTDAT   (*(volatile unsigned *)_rWTDAT)	//Watch-dog timer data
#define rWTCNT   (*(volatile unsigned *)_rWTCNT)	//Eatch-dog timer count

//定时器和电源相关寄存器
// CLOCK & POWER MANAGEMENT
#define _rLOCKTIME   0x4c000000
#define _rMPLLCON    0x4c000004
#define _rUPLLCON    0x4c000008
#define _rCLKCON     0x4c00000c
#define _rCLKSLOW    0x4c000010
#define _rCLKDIVN    0x4c000014
#define _rCAMDIVN    0x4c000018

#define rLOCKTIME   (*(volatile unsigned *)_rLOCKTIME)	//PLL lock time counter
#define rMPLLCON    (*(volatile unsigned *)_rMPLLCON)	//MPLL Control
#define rUPLLCON    (*(volatile unsigned *)_rUPLLCON)	//UPLL Control
#define rCLKCON     (*(volatile unsigned *)_rCLKCON)	//Clock generator control
#define rCLKSLOW    (*(volatile unsigned *)_rCLKSLOW)	//Slow clock control
#define rCLKDIVN    (*(volatile unsigned *)_rCLKDIVN)	//Clock divider control
#define rCAMDIVN    (*(volatile unsigned *)_rCAMDIVN)	//USB, CAM Clock divider control

#define rLOCKTIME_INIT 0xffffffff   //大于300us即可
#define rCLKDIVN_INIT ((0x2 << 1) | (1 << 0))   //FCLK:HCLK:PCLK=4:1:0.5 ,需要设置进入异步总线模式
//UPLL=48mhz
#define U_MDIV  0x38
#define U_PDIV  0x2
#define U_SDIV  0x2
//FLCK = MPLL = 400Mhz(数据手册中没有准确的400mhz，浏览器搜索获得) 
#define M_MDIV  0x5c
#define M_PDIV  0x1
#define M_SDIV  0x1

//中断相关寄存器
// INTERRUPT
#define _rSRCPND     0x4a000000
#define _rINTMOD     0x4a000004
#define _rINTMSK     0x4a000008
#define _rPRIORITY   0x4a00000c
#define _rINTPND     0x4a000010
#define _rINTOFFSET  0x4a000014
#define _rSUBSRCPND  0x4a000018
#define _rINTSUBMSK  0x4a00001c

#define rSRCPND     (*(volatile unsigned *)_rSRCPND)	//中断源未决寄存器
#define rINTMOD     (*(volatile unsigned *)_rINTMOD)	//Interrupt mode control
#define rINTMSK     (*(volatile unsigned *)_rINTMSK)	//Interrupt mask control
#define rPRIORITY   (*(volatile unsigned *)_rPRIORITY)	//IRQ priority control
#define rINTPND     (*(volatile unsigned *)_rINTPND)	//中断未决寄存器
#define rINTOFFSET  (*(volatile unsigned *)_rINTOFFSET)	//Interruot request source offset
#define rSUBSRCPND  (*(volatile unsigned *)_rSUBSRCPND)	//Sub source pending
#define rINTSUBMSK  (*(volatile unsigned *)_rINTSUBMSK)	//Interrupt sub mask


//SDRAM相关寄存器
// Memory control 
#define _rBWSCON    0x48000000
#define _rBANKCON0  0x48000004
#define _rREFRESH   0x48000024
#define _rBANKSIZE  0x48000028
#define _rMRSRB6    0x4800002c
#define _rMRSRB7    0x48000030

#define rBWSCON    (*(volatile unsigned *)_rBWSCON)	//Bus width & wait 状态寄存器
#define rBANKCON0  (*(volatile unsigned *)_rBANKCON0)	//Boot ROM control
#define rBANKCON1  (*(volatile unsigned *)0x48000008)	//BANK1 control
#define rBANKCON2  (*(volatile unsigned *)0x4800000c)	//BANK2 cControl
#define rBANKCON3  (*(volatile unsigned *)0x48000010)	//BANK3 control
#define rBANKCON4  (*(volatile unsigned *)0x48000014)	//BANK4 control
#define rBANKCON5  (*(volatile unsigned *)0x48000018)	//BANK5 control
#define rBANKCON6  (*(volatile unsigned *)0x4800001c)	//BANK6 control
#define rBANKCON7  (*(volatile unsigned *)0x48000020)	//BANK7 control
#define rREFRESH   (*(volatile unsigned *)0x48000024)	//DRAM/SDRAM refresh
#define rBANKSIZE  (*(volatile unsigned *)0x48000028)	//Flexible Bank Size
#define rMRSRB6    (*(volatile unsigned *)0x4800002c)	//Mode register set for SDRAM
#define rMRSRB7    (*(volatile unsigned *)0x48000030)	//Mode register set for SDRAM

//bank1~bank7每4位控制一个bank 4位由高到低分别是ST,WS,DW(占两位)
//ST（启动/禁止SDRAM的数据掩码引脚。对于SDRAM，此位置0；对于SRAM，此位置1）
//WS（是否使用存储器的WAIT信号，通常置0为不使用）
//DW（两位，设置位宽。此板子的SDRAM是32位，故将DW6设为10）
#define rBWSCON_INIT 0x22000008

//bank控制寄存器相关位设置
#define rBANKCON_0_5_INIT    (0x0700)
#define rBANKCON6_INIT   ((0x3 << 15) | (0x1 << 2) | (0x1 << 0)) //banck6,7为SDRAM且有9-bit列地址位数
#define rBANKCON7_INIT   ((0x3 << 15) | (0x1 << 2) | (0x1 << 0)) 
//刷新控制寄存器设置
#define rREFRESH_INIT  ((0x1 << 23) | (0x1 << 20) | (0x2 << 18) | 1268) //((1 << 11) + 1 - SDRAM_REFRESH_TIME * OS_HCLK / 1e6)

//BANKSIZE寄存器设置
#define rBANKSIZE_INIT  ((0x1 << 7) | (0x1 << 5) | (0x1 << 4) | (0x1 << 0))

//SDRAM模式寄存器设置
#define rMRSRB_INIT   (0x3 << 4)

//NANDFLASH相关寄存器
#define rNFCONF		(*(volatile unsigned *)0x4E000000)		//NAND Flash configuration
#define rNFCONT		(*(volatile unsigned *)0x4E000004)      //NAND Flash control
#define rNFCMD		(*(volatile unsigned *)0x4E000008)      //NAND Flash command
#define rNFADDR		(*(volatile unsigned *)0x4E00000C)      //NAND Flash address
#define rNFDATA		(*(volatile unsigned *)0x4E000010)      //NAND Flash data
#define rNFDATA8	(*(volatile unsigned char *)0x4E000010)     //NAND Flash data

#define rNFMECCD0	(*(volatile unsigned *)0x4E000014)      //NAND Flash ECC for Main Area
#define rNFMECCD1	(*(volatile unsigned *)0x4E000018)
#define rNFSECCD	(*(volatile unsigned *)0x4E00001C)		//NAND Flash ECC for Spare Area
#define rNFSTAT		(*(volatile unsigned *)0x4E000020)		//NAND Flash operation status
#define rNFESTAT0	(*(volatile unsigned *)0x4E000024)
#define rNFESTAT1	(*(volatile unsigned *)0x4E000028)
#define rNFMECC0	(*(volatile unsigned *)0x4E00002C)
#define rNFMECC1	(*(volatile unsigned *)0x4E000030)
#define rNFSECC		(*(volatile unsigned *)0x4E000034)
#define rNFSBLK		(*(volatile unsigned *)0x4E000038)		//NAND Flash Start block address
#define rNFEBLK		(*(volatile unsigned *)0x4E00003C)		//NAND Flash End block address

#endif