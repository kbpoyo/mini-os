#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "common/types.h"

__attribute__((always_inline)) static uint32_t cpu_get_cpser(void) {
  uint32_t ret = 0;

  __asm__ __volatile__("mrs %[ret], cpsr\n" : [ret] "=r"(ret) : : "r0");

  return ret;
}

__attribute__((always_inline)) static void cpu_set_cpsr(uint32_t state) {
  __asm__ __volatile__("msr cpsr, %[state]\n" : : [state] "r"(state) : "r0");
}

__attribute__((always_inline)) static uint32_t cpu_get_spser(void) {
  uint32_t ret = 0;

  __asm__ __volatile__("mrs %[ret], spsr\n" : [ret] "=r"(ret) : : "r0");

  return ret;
}

__attribute__((always_inline)) static void cpu_set_spsr(uint32_t state) {
  __asm__ __volatile__("msr spsr, %[state]\n" : : [state] "r"(state) : "r0");
}

/**
 * @brief 开中断
 *
 */
__attribute__((always_inline)) static void cpu_irq_start() {
  __asm__ __volatile__(
      "mrs r0, cpsr\n"
      "bic r0, #0x80\n"
      "msr cpsr, r0\n" ::
          : "r0", "memory");
}

/**
 * @brief 关中断
 *
 */
__attribute__((always_inline)) static void cpu_irq_close() {
  __asm__ __volatile__(
      "mrs r0, cpsr\n"
      "orr r0, #0x80\n"
      "msr cpsr, r0\n" ::
          : "r0");
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
      : [svc_sp] "r"(svc_sp)
      : "r0");
}

#define cpu_cp15_read(cr, op1, op2, ret)                                \
  __asm__ __volatile__("mrc p15," #op1 ",%[" #ret "]," #cr ",cr0," #op2 \
                       : [ret] "=r"(ret)::"memory")

#define cpu_cp15_write(cr, op1, op2, arg)                  \
  __asm__ __volatile__("mcr p15," #op1 ",%[" #arg "]," #cr \
                       ",cr0," #op2 ::[arg] "r"(arg)       \
                       : "memory")

/**
 * @brief 读取cr0寄存器，即存储标识符寄存器，记录了cache相关工作属性
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr0_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr0, 0, 1, ret);

  return ret;
}

/**
 * @brief 读取cr1寄存器，即存储控制寄存器
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr1_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr1, 0, 0, ret);
  return ret;
}

/**
 * @brief 写入cr1寄存器
 *
 */
__attribute__((always_inline)) static void cpu_cr1_write(uint32_t arg) {
  cpu_cp15_write(cr1, 0, 0, arg);
}

/**
 * @brief 读取cr2寄存器，即页目录表基地址寄存器
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr2_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr2, 0, 0, ret);
  return ret;
}

/**
 * @brief 写入cr2寄存器
 *
 */
__attribute__((always_inline)) static void cpu_cr2_write(uint32_t arg) {
  cpu_cp15_write(cr2, 0, 0, arg);
}

/**
 * @brief 读取cr3寄存器，即空间域控制寄存器
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr3_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr3, 0, 0, ret);
  return ret;
}

/**
 * @brief 写入cr3寄存器
 *
 */
__attribute__((always_inline)) static void cpu_cr3_write(uint32_t arg) {
  cpu_cp15_write(cr3, 0, 0, arg);
}

/**
 * @brief 读取cr5,即失效状态寄存器
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr5_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr5, 0, 0, ret);
  return ret;
}

/**
 * @brief 读取cr6，即失效地址寄存器
 *
 */
__attribute__((always_inline)) static uint32_t cpu_cr6_read() {
  uint32_t ret = 0;
  cpu_cp15_read(cr6, 0, 0, ret);
  return ret;
}

#endif