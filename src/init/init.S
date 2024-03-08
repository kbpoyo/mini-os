
#include "common/register_addr.h"
#include "common/os_config.h" 
    .text
    .global _start

_start:
    b _exc_handler_reset
    b _exc_handler_undef
    b _exc_handler_swi
    b _exc_handler_prefetch_abort
    b _exc_handler_data_abort
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

    bl _init_stack

    ldr r0, =EXCEPTION_RESET  //标志当前处理在处理rest异常
    ldr pc, =KERNEL_ADDR

_init_stack:
    //初始化特权模式的栈空间
    ldr sp, =STACK_ADDR_SVC   //svc模式
    mrs r1, cpsr    //保存svc模式下的cpsr

    mrs r0, cpsr
    bic r0, #0x1f
    orr r0, #CPU_MODE_FIQ   //将当前状态设置为FIQ模式，并初始化栈空间
    msr cpsr, r0
    ldr sp, =STACK_ADDR_FIQ

    bic r0, #0x1f
    orr r0, #CPU_MODE_IRQ
    msr cpsr, r0    //将当前状态设置为IRQ模式，并初始化栈空间
    ldr sp, =STACK_ADDR_IRQ 

    bic r0, #0x1f
    orr r0, #CPU_MODE_ABT    //将当前状态设置为Abort模式，并初始化栈空间
    msr cpsr, r0
    ldr sp, =STACK_ADDR_ABORT

    bic r0, #0x1f
    orr r0, #CPU_MODE_UND    //将当前状态设置为UND模式，并初始化栈空间
    msr cpsr, r0
    ldr sp, =STACK_ADDR_UND

    bic r0, #0x1f
    orr r0, #CPU_MODE_SYS    //将当前状态设置为SYS或者USER模式，并初始化栈空间
    msr cpsr, r0
    ldr sp, =STACK_ADDR_SYS_AND_USER

    msr cpsr, r1    //恢复到svc模式

    mov pc, lr





_exc_handler_swi:

//保存返回地址
    push {lr}
//保存用户模式的寄存器组
    stmfd sp, {r0-r14}^
    nop
    sub sp, #60


//保存用户模式的状态寄存器
    mrs r0, spsr
    push {r0}

    ldr r0, =EXCEPTION_SWI
    ldr pc, =KERNEL_ADDR

_exc_handler_irq:
    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)    //进入svc模式，在内核栈中保存r0-r12,lr
    sub sp, #4
    stmfd sp!, {r0-r12, lr} //保存svc模式的lr,从而可以在svc模式下响应中断时，lr不被破坏
    nop

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_IRQ)    //回到irq模式转移返回地址和spsr的值
    sub r0, lr, #4
    mrs r1, spsr

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)    //再进入svc模式，在内核栈中保存返回地址，并设置svc模式的spsr
    str r0, [sp, #56]
    msr spsr, r1    //如果想打开中断嵌套，需要再保存一份spsr到内核栈中，这里我暂时不开中断嵌套

    ldr r0, =EXCEPTION_IRQ
    ldr pc, =KERNEL_ADDR




//对于stm和ldm指令，当pc不在传输列表时,且使用^将s位置位
//表示传输的是用户模式寄存器，且不能使用基地址回写
//当pc在传输列表时，对应ldm指令可以使用基地址回写，通常用来
//异常模式下返回
_exc_handler_data_abort:
    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    stmfd sp, {r0-r14}^ 
    nop
    sub sp, #60

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_ABT)   //回到abt模式转移r14和spsr
    sub r0, r14, #8
    mrs r1, spsr

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    push {r0}
    msr spsr, r1

    ldr r0, =EXCEPTION_DATA_ABORT
    ldr pc, =KERNEL_ADDR

_exc_handler_prefetch_abort:
    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    stmfd sp, {r0-r14}^ 
    nop
    sub sp, #60

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_ABT)   //回到abt模式转移r14和spsr
    sub r0, r14, #4
    mrs r1, spsr

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    push {r0}
    msr spsr, r1

    ldr r0, =EXCEPTION_PREFETCH_ABORT
    ldr pc, =KERNEL_ADDR
    

_exc_handler_undef:
    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    stmfd sp, {r0-r14}^ 
    nop
    sub sp, #60

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_UND)   //回到abt模式转移r14和spsr
    mov r0, r14
    mrs r1, spsr

    msr cpsr_c, (CPU_MASK_IRQ | CPU_MODE_SVC)   //进入svc模式使用内核栈
    push {r0}
    msr spsr, r1

    ldr r0, =EXCEPTION_UNDEF
    ldr pc, =KERNEL_ADDR


_exc_handler_ifq:
    bl _start
undef:
    .word 0xeeadc0de