#include "core/sys_exception.h"

#include "common/cpu_instr.h"
#include "tools/log.h"

void print_exception_frame_info(exception_frame_t* frame) {
  log_printf(
      "r0:\t0x%x\n"
      "r1:\t0x%x\n"
      "r2:\t0x%x\n"
      "r3:\t0x%x\n"
      "r4:\t0x%x\n"
      "r5:\t0x%x\n"
      "r6:\t0x%x\n"
      "r7:\t0x%x\n"
      "r8:\t0x%x\n"
      "r9:\t0x%x\n"
      "r10:\t0x%x\n"
      "r11(fl):\t0x%x\n"
      "r12:\t0x%x\n"
      "r13(sp):\t0x%x\n"
      "r14(lr):\t0x%x\n"
      "cpsr:\t0x%x\n",
      frame->r0, frame->r1, frame->r2, frame->r3, frame->r4, frame->r5,
      frame->r6, frame->r7, frame->r8, frame->r9, frame->r10, frame->r11,
      frame->r12, frame->r13, frame->r14, cpu_get_spser());
}

void undef_handler(exception_frame_t* frame) {
  log_printf(
      "==================== Task Error ====================\n"
      "Undef Error:\n");
  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  while (1) {
  }
}

static void print_mmu_err_state() {
  uint32_t mmu_err_code = cpu_cr5_read();

  log_printf("mmu err info:\n");

  if ((mmu_err_code & 0xf) == MMU_ERR_FIRST_PAGE_ENTRY) {
    log_printf(
        ESC_COLOR_ERROR
        "error: first level page access error, the first page table entry is "
        "not "
        "present.\n" ESC_COLOR_DEFAULT);
  } else if ((mmu_err_code & 0xf) == MMU_ERR_SECOND_PAGE_ENTRY) {
    log_printf(ESC_COLOR_ERROR
               "error: second level page access error, the second page table "
               "entry is "
               "not present.\n" ESC_COLOR_DEFAULT);
  } else if ((mmu_err_code & 0xf) == MMU_ERR_PAGE_ACCESS) {
    log_printf(ESC_COLOR_ERROR
               "error: Page access is not authorized!\n" ESC_COLOR_DEFAULT);
  } else {
    log_printf(ESC_COLOR_ERROR "err: unknown err.\n" ESC_COLOR_DEFAULT);
  }
}

void data_abort_handler(exception_frame_t* frame) {
  log_printf(
      "==================== Task Error ====================\n"
      "Data Abort Error:\n"
      "error address:\t0x%x\n"
      "error state:\t0x%x\n",
      cpu_cr6_read(), cpu_cr5_read());

  print_mmu_err_state();

  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  while (1) {
  }
}

void prefetch_abort_handler(exception_frame_t* frame) {
  log_printf(
      "==================== Task Error ====================\n"
      "Prefetch Abort Error:\n"
      "error address:\t0x%x\n"
      "error state:\t0x%x\n",
      cpu_cr6_read(), cpu_cr5_read());

  print_mmu_err_state();

  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  while (1) {
  }
}
