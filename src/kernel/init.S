
#include "common/register_addr.h"
#include "common/os_config.h"

    .text
    .global _kernel_init
    .global _exception_handler
    .global task_switch_by_sp
    .extern irq_handler



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


    bl kernel_init




_irq_handler:

    bl irq_handler
    //恢复cpu上下文
    ldmfd sp!, {r0-r12, pc}^



task_switch_by_sp:
    //保存cpu上下文环境, r0存储当前任务sp，r1存储目标任务sp
    push {r1}
    mrs r1, cpsr
    sub r0, #4
    str r1, [r0]
    pop {r1}
    sub r1, #64
    stmia r0, {r0-r15}

    //切换任务栈空间
    mov sp, r1

    //恢复新任务的cpu上下
    pop {r1}
    msr cpsr, r1
    ldmfd sp!, {r0-r15}
