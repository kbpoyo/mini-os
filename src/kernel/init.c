
#include "common/cpu_instr.h"
#include "core/dev.h"
#include "core/irq.h"
#include "core/memory.h"
#include "core/task.h"
#include "dev/gpio.h"
#include "dev/nandflash.h"
#include "dev/timer.h"
#include "dev/uart.h"
#include "fs/fs.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "dev/sd.h"

void delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 400; j++)
      ;
  }
}

uint32_t num __attribute__((section(".data"), aligned(16))) = 0;

uint32_t stack_1[1024] __attribute__((section(".data"), aligned(16))) = {0};
void task_test_1(void) {
  while (1) {
    // if (num <= 1000) {
    //   yield();
    // }
    log_printf("task_1 runing!, TTTT num = %d\n", num++);
    msleep(1000);
  }
}

uint32_t stack_2[1024] __attribute__((section(".data"), aligned(16))) = {0};
void task_test_2(void) {
  while (1) {
    log_printf("task_2 runing! num = %d\n", num++);
    msleep(1000);
  }
}

void write_fs_nand() {
  char* buff = (char*)0x30200000;

  int sector_count = 64 * 1024 * 1024 / 512;

  int write_count = 64 * 4;

  disk_t *disk;
  nand_open(disk);

  for (int i = 0; i < sector_count; i += write_count) {
    int ret = nand_write(disk, i, buff, write_count);
    buff += 512 * write_count;
    log_printf("write block: %d\n", i);
  }

  nand_close(disk);
}

/**
 * @brief 跳转到第一个任务进程
 *
 */
void move_to_first_task(void) {
  // 1.获取当前任务
  task_t* curr = task_current();
  ASSERT(curr != 0);

  // 2.获取当前任务的tss结构
  register_group_t* reg = &(curr->reg_group);

  // 设置用户栈和内核栈并进行任务跳转且切换模式到用户模式
  __asm__ __volatile__(
      "mov sp, %[user_sp]\n"
      "msr cpsr, %[svc_cpsr]\n"
      "mov sp, %[svc_sp]\n"
      "msr spsr, %[user_cpsr]\n"
      "push {%[entry]}\n"
      "ldmfd sp!, {pc}^\n" ::[user_sp] "r"(reg->r13),
      [svc_cpsr] "r"(CPU_MODE_SVC), [svc_sp] "r"(curr->task_sw.svc_sp),
      [user_cpsr] "r"(CPU_MODE_USER), [entry] "r"(reg->r15));
}

int kernel_init() {
  gpio_init();

  log_init();

  irq_init();

  // write_fs_nand();

  memory_init();

  task_manager_init();

  fs_init();

  task_first_init();

  timer_init();

  cpu_irq_start();

  log_printf("kbos version: " OS_VERSION "\n");

  move_to_first_task();

  while (1)
    ;
}