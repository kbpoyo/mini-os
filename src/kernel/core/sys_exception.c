#include "core/sys_exception.h"

#include "common/cpu_instr.h"
#include "core/task.h"
#include "tools/log.h"

void print_exception_frame_info(exception_frame_t* frame) {
  log_printf(
      "user register group:\n"
      "\tr0:\t\t0x%x\n"
      "\tr1:\t\t0x%x\n"
      "\tr2:\t\t0x%x\n"
      "\tr3:\t\t0x%x\n"
      "\tr4:\t\t0x%x\n"
      "\tr5:\t\t0x%x\n"
      "\tr6:\t\t0x%x\n"
      "\tr7:\t\t0x%x\n"
      "\tr8:\t\t0x%x\n"
      "\tr9:\t\t0x%x\n"
      "\tr10:\t\t0x%x\n"
      "\tr11(fl):\t0x%x\n"
      "\tr12:\t\t0x%x\n"
      "\tr13(sp):\t0x%x\n"
      "\tr14(lr):\t0x%x\n"
      "\tcpsr:\t\t0x%x\n",
      frame->r0, frame->r1, frame->r2, frame->r3, frame->r4, frame->r5,
      frame->r6, frame->r7, frame->r8, frame->r9, frame->r10, frame->r11,
      frame->r12, frame->r13, frame->r14, cpu_get_spser());
}

void undef_handler(exception_frame_t* frame) {
  log_printf(
      "==================== Task Error ====================\n"
      "Undef Error:\n"
      "error instr address:\t0x%x\n",
      frame->err_addr);
  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  sys_exit(-1);
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
      "error instr address:\t0x%x\n"
      "error access address:\t0x%x\n"
      "error state:\t0x%x\n",
      frame->err_addr, cpu_cr6_read(), cpu_cr5_read());

  print_mmu_err_state();

  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  sys_exit(-1);
}

void prefetch_abort_handler(exception_frame_t* frame) {
  log_printf(
      "==================== Task Error ====================\n"
      "Prefetch Abort Error:\n"
      "error instr address:\t0x%x\n"
      "error access address:\t0x%x\n"
      "error state:\t0x%x\n",
      frame->err_addr, cpu_cr6_read(), cpu_cr5_read());

  print_mmu_err_state();

  print_exception_frame_info(frame);
  log_printf("===================================================\n");

  sys_exit(-1);
}
