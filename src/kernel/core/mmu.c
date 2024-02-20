#include "core/mmu.h"

#include "common/cpu_instr.h"
#include "tools/log.h"

void do_undef(void) {
  __asm__ __volatile__(".word 0xeeadc0de\n");  // 异常指令
}

void enable_mmu() {
  uint32_t cr1 = cpu_cr1_read();
  uint32_t cr3 = cpu_cr3_read();

  // 将cr1的[0]位置位从而使能mmu, 并打开指令cache和数据cache
  cr1 |= CR1_MMU_ENABEL | CR1_INSTR_CACHE_ENABLE | CR1_DATA_CACHE_ENABLE;
  //  设置D0域的权限控制为客户模式，将权限将给页表项进行检测
  cr3 &= 0xfffffffc;
  cr3 |= CR3_D0;

  cpu_cr3_write(cr3);
  cpu_cr1_write(cr1);
}