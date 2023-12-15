
#include "common/register_addr.h"
    
    .text
    .global _start

_start:
    b _exc_handler_reset
    b _exc_handler_undef
    b _exc_handler_swi
    b _exc_handler_abort
    nop
    b _exc_handler_irq
    b _exc_handler_ifq



_exc_handler_reset:
    //1.关闭看门狗
    ldr r0, =_rWTCON
    ldr r1, =0x0
    str r1, [r0]

    //2.屏蔽所有中断
    ldr r0, =_rINTMSK
    ldr r1, =0xffffffff
    str r1, [r0]

	ldr	r0,=_rINTSUBMSK    //屏蔽所有子中断
	ldr	r1,=0x7fff
	str	r1,[r0]


    //3.初始化锁相环(PLL)和时钟
    ldr r0, =_rLOCKTIME        //设置M_PLL和U_PLL的生效延迟
    ldr r1, =rLOCKTIME_INIT    //由于配置或其它原因导致主频变化时
    str r1, [r0]                //PLL新的输出需要一个稳定的过度时间

    ldr r0, =_rCLKDIVN //设置时钟分配控制寄存器，FCLK:HCLK:PCLK=4:1:0.5
    ldr r1, =rCLKDIVN_INIT
    str r1, [r0]

    ldr r0, =_rCAMDIVN  //设置摄像头时钟分频控制寄存器，配合rCLKDIVN使用
    ldr r1, =rCAMDIVN_INIT
    str r1, [r0]

    mrc p15, 0, r0, c1, c0, 0   //设置协处理器的c1寄存器，使cpu进入异步总线模式，不然cpu使用的是HCLK而不是FCLK
    orr r0, r0, #0xc0000000
    mcr p15, 0, r0, c1, c0, 0


    ldr r0, =_rUPLLCON //需要先设置UPLL再设置MPLL
    ldr r1, =((U_MDIV << 12) + (U_PDIV << 4) + U_SDIV)
    str r1, [r0]
    nop //需要7个时钟等待upll生效
    nop
    nop
    nop
    nop
    nop
    nop

    ldr r0, =_rMPLLCON  //设置MPLL
    ldr r1, =((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV)
    str r1, [r0]

    ldr r0, =_rCLKCON   //初始化外设时钟使能控制寄存器
    ldr r1, =rCLKCON_INIT
    str r1, [r0]

    //4.初始化SDRAM
    ldr r0, =_rBWSCON   //初始化各个块的总线宽度和wait状态
    ldr r1, =rBWSCON_INIT
    str r1, [r0]

    ldr r0, =_rBANKCON0 //初始化bank0~5
    ldr r1, =rBANKCON_0_5_INIT
    mov r2, r1
    mov r3, r1
    mov r4, r1
    mov r5, r1
    mov r6, r1
    stmia r0!, {r1-r6}

    ldr r0, =(_rBANKCON0 + (6 << 2))  //初始化bank6和bank7
    ldr r1, =rBANKCON6_INIT
    str r1, [r0]

    ldr r0, =(_rBANKCON0 + (7 << 2))
    ldr r1, =rBANKCON7_INIT
    str r1, [r0]

    ldr r0, =_rREFRESH  //初始化刷新控制寄存器
    ldr r1, =rREFRESH_INIT
    str r1, [r0]

    ldr r0, =_rBANKSIZE //初始化banksize寄存器
    ldr r1, =rBANKSIZE_INIT
    str r1, [r0]

    ldr r0, =_rMRSRB6   //初始化SDRAM模式寄存器，设置CAS
    ldr r1, =_rMRSRB7
    ldr r2, =rMRSRB_INIT
    str r2, [r0]
    str r2, [r1]

    ldr sp, =0x34000000
    ldr pc, =KERNEL_INIT_ADDR

    


_exc_handler_undef:
    ldr r0, undef
    swi #0
    bl _start

_exc_handler_swi:
    bl _start

_exc_handler_abort:
    bl _start

_exc_handler_irq:
    bl _start

_exc_handler_ifq:
    bl _start
undef:
    .word 0xeeadc0de