
#include "common/register_addr.h"
#include "common/os_config.h"

    .text
    .global _kernel_init
    .global _exception_handler
    .extern irq_handler

#define EXCEPTION_RESET 0
#define EXCEPTION_UNDEF 1
#define EXCEPTION_SWI   2
#define EXCEPTION_PREFETCH_ABORT 3
#define EXCEPTION_DATA_ABORT 4
#define EXCEPTION_IRQ   5
#define EXCEPTION_FIQ   6


_exception_handler:
    
    cmp r0, #EXCEPTION_IRQ
    beq _irq_handler
   
   cmp r0, #EXCEPTION_RESET
   beq _kernel_init

 



_kernel_init:
    mov r0, #0
    mov r1, r0
    mov r2, r0
    mov r3, r0
    mov r4, r0
    mov r5, r0
    mov r6, r0
    mov r7, r0
    mov r8, r0
    mov r9, r0
    mov r10, r0
    mov r11, r0
    mov r12, r0
    mov r14, r0
    
    //打开中断
    mrs r0, cpsr
    bic r0, #0xc0
    msr cpsr_c, r0


    bl kernel_init




_irq_handler:

    //保存cpu上下文
    bl irq_handler
    //恢复cpu上下文
    ldmfd sp!, {r0-r12, pc}^