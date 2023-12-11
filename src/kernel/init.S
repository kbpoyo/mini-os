

    .text
    .global _start

_start:
    //关中断
    mrs r0, cpsr
    orr r0, r0, #0x00df
    msr cpsr_c, r0
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
    ldr r13, =0x34000000
    
    bl kernel_init
