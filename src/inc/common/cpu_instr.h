#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "common/types.h"

__attribute__((always_inline)) static uint32_t cpu_get_cpser(void) {
    
    uint32_t ret = 0;

    __asm__ __volatile__(
        "mrs %[ret], cpsr\n"
        :[ret]"=r"(ret)
        :
        :"r0"
    );

    return ret;
}

__attribute__((always_inline)) static void cpu_set_cpsr(uint32_t state) {
    __asm__ __volatile__(
        "msr cpsr, %[state]\n"
        :
        :[state]"r"(state)
        :"r0"
    );  
}


/**
 * @brief 开中断
 *
 */
__attribute__((always_inline)) static void cpu_irq_start() {
    __asm__ __volatile__(
        "mrs r0, cpsr\n"
        "bic r0, #0x80\n"
        "msr cpsr, r0\n"
        :::"r0","memory"
    );
}

/**
 * @brief 关中断
 *
 */
__attribute__((always_inline)) static void cpu_irq_close() {
       __asm__ __volatile__(
        "mrs r0, cpsr\n"
        "orr r0, #0x80\n"
        "msr cpsr, r0\n"
        :::"r0"
    );
}

__attribute__((always_inline)) static void cpu_set_irq_stack(uint32_t svc_sp) {
       __asm__ __volatile__(
        "mrs r0, cpsr\n"
        "bic r0, #0x1f\n"
        "orr r0, #0x12\n"
        "msr cpsr, r0\n"
        "mov sp, %[svc_sp]\n"
        "mrs r0, cpsr\n"
        "orr r0, #0x1f\n"
        "msr cpsr, r0\n"
        :
        :[svc_sp]"r"(svc_sp)
        :"r0"
    );
}







#endif