
#include "common/register_addr.h"
#include "common/os_config.h"

    .text
    .global _kernel_init
    .global _exception_handler
    .global task_switch_by_sp
    .extern irq_handler
    .extern swi_handler



_exception_handler:
    
  
    mov r1, pc;
    ldr pc, [r1, r0, lsl#2]

    //异常任务分发数组，顺序固定
    .word   _kernel_init    //reset 异常
    .word   _undef_handler
    .word   _swi_handler
    .word   _prefetch_abort_handler
    .word   _data_abort_handler
    .word   _irq_handler
    .word   _fiq_handler


 



_kernel_init:
    //设置系统模式
    mrs r0, cpsr
    bic r0, #0x1f
    orr r0, #(CPU_MASK_IRQ | CPU_MODE_SYS)
    msr cpsr, r0

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




_swi_handler:
    //打开中断
    msr cpsr_c, #CPU_MODE_SVC

    //传入系统调用栈帧
    mov r0, sp
    bl swi_handler

    //关闭中断
    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)
    //恢复cpu上下文
    pop {r0}
    msr spsr, r0
    ldmfd sp, {r0-r14}^ //恢复用户组寄存器
    nop
    add sp, #60

    ldmfd sp!, {pc}^    //加载返回地址



_irq_handler:

    bl irq_handler
    //恢复cpu上下文
    ldmfd sp!, {r0-r12,lr, pc}^

_loop:
    bl _loop

_undef_handler:
    mov r0, sp;
    bl undef_handler
    bl _loop
   
    

_data_abort_handler:

    mov r0, sp;
    bl data_abort_handler
    bl _loop
   

_prefetch_abort_handler:
    mov r0, sp;
    bl prefetch_abort_handler
    bl _loop
   

_fiq_handler:



task_switch_by_sp:
    //保存cpu上下文环境, r0指向的内存单元存储当前任务的svc_sp，r1指向的内存单元存储目标任务svc_sp
    //在当前任务的内核栈中保存上下文
    push {lr}   //保存返回地址
    stmfd sp, {r0-r14}^    //^:保护用户状态的下的r0-r14
    nop //上一条指令后面不能紧跟访问备份寄存器的指令，最后跟一条nop
    sub sp, #60

    mrs r2, cpsr    //保存状态寄存器
    push {r2}       

    mrs r2, spsr    //保存状态备份寄存器
    push {r2}

    //切换任务的svc_sp
    str sp, [r0]    //更新当前的任务的svc_sp值
    //切换栈空间
    ldr sp, [r1]    //加载目标任务的svc_sp值

    //读取新页目录表
    ldr r2, [r1, #4]
    mov r0, #0
    
    mcr p15, 0, r2, c2, c0, 0 //切换页目录表
    mcr p15, 0, r0, c8, c7, 0   //！！使无效整个TLB 



    //恢复新任务的cpu上下文
    pop {r0}
    msr spsr, r0

    //读取cpsr
    pop {r0}

    //中断位未置1,说明是任务第一次启动
    tst r0, #0x80 
    beq task_switch_to_start

    msr cpsr, r0    

    ldmfd sp, {r0-r14}^    //^:恢复用户模式的寄存器组
    nop
    add sp, #60
    ldmfd sp!, {pc}  //切换完毕，返回调用者处的下一条待执行指令处执行

task_switch_to_start:
    ldmfd sp, {r0-r14}^
    nop
    add sp, #60
    ldmfd sp!, {pc}^ //新任务执行，将spsr传入cpsr
