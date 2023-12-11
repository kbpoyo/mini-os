
#include "common/register_addr.h"
    
    .text
    .global _start

_start:
    bl exc_handler_reset
    bl exc_handler_undef
    bl exc_handler_swi
    bl exc_handler_abort
    .word 0x0
    bl exc_handler_irq
    bl exc_handler_ifq



exc_handler_reset:
    mov r0, #WTDCON
    ldr pc, =undef


exc_handler_undef:
    ldr r0, undef
    swi #0
    bl _start

exc_handler_swi:
    bl _start

exc_handler_abort:
    bl _start

exc_handler_irq:
    bl _start

exc_handler_ifq:
    bl _start
undef:
    .word 0xeeadc0de