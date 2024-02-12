#include "core/mmu.h"

#include "common/cpu_instr.h"
#include "tools/log.h"

uint32_t pde[1024];

void enable_mmu() {
  uint32_t cr1 = cpu_cr1_read();
  uint32_t cr2 = cpu_cr2_read();
  log_printf("cr1 = 0x%x, cr2 = 0x%x\n", cr1, cr2);

  cr1 |= CR1_MMU_ENABEL;

  cpu_cr2_write(pde);
  cpu_cr1_write(cr1);

  cr2 = cpu_cr2_read();
  cr1 = cpu_cr1_read();
  log_printf("cr1 = 0x%x, cr2 = 0x%x\n", cr1, cr2);

  while (1) {
  };
}