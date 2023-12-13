
#include "common/register_addr.h"
    
    .text
    .global _start

_start:
    bl _exc_handler_reset
    bl _exc_handler_undef
    bl _exc_handler_swi
    bl _exc_handler_abort
    .word 0x0
    bl _exc_handler_irq
    bl _exc_handler_ifq



_exc_handler_reset:
    //1.关闭看门狗
    ldr r0, =#_rWTCON
    ldr r1, #0x0
    str r1, [r0]

    //2.屏蔽所有中断
    ldr r0, =#_rINTMSK
    ldr r1, =0xffffffff
    str r1, [r0]

    //3.初始化锁相环(PLL)和时钟
    
    
   


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