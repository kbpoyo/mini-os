#include "core/mmu.h"

#include "common/cpu_instr.h"
#include "tools/log.h"

void do_undef(void) {
  __asm__ __volatile__(".word 0xeeadc0de\n");  // 异常指令
}

void enable_mmu(uint32_t kernel_page_dir) {
  uint32_t cr1 = cpu_cr1_read();
  uint32_t cr2 = cpu_cr2_read();
  uint32_t cr3 = cpu_cr3_read();
  log_printf("cr1 = 0x%x, cr2 = 0x%x, cr3 = 0x%x\n", cr1, cr2, cr3);

  cr1 |= CR1_MMU_ENABEL;
  cr3 |= CR3_D0;

  cpu_cr3_write(cr3);
  cpu_cr2_write(kernel_page_dir);
  cpu_cr1_write(cr1);

  cr2 = cpu_cr2_read();
  cr1 = cpu_cr1_read();

  log_printf("cr1 = 0x%x, cr2 = 0x%x\n", cr1, cr2);

  do_undef();
  int num = 10;
  while (1) {
    num++;
  };
}