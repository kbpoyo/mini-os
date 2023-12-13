#ifndef _REGISTER_ADDR_H_
#define _REGISTER_ADDR_H_

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



#endif