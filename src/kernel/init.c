
#include "applib/lib_syscall.h"
#include "common/cpu_instr.h"
#include "core/irq.h"
#include "core/memory.h"
#include "core/task.h"
#include "dev/gpio.h"
#include "dev/nandflash.h"
#include "dev/timer.h"
#include "dev/uart.h"
#include "tools/klib.h"
#include "tools/log.h"

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

static uint8_t block_buff[2 * 64 * 4][512];
__attribute__((section(".data"), align(4 * 1024)));

int kernel_init() {
  gpio_init();

  log_init();

  irq_init();

  // memory_init();

  // task_manager_init();

  // task_first_init();

  // task_t* task_1 = task_alloc();
  // int ret = task_init(task_1, "task_1", (uint32_t)task_test_1,
  //                     (uint32_t)&stack_1[1024], TASK_FLAGS_SYSTEM);
  // ASSERT(ret != -1);

  // task_t* task_2 = task_alloc();
  // ret = task_init(task_2, "task_2", (uint32_t)task_test_2,
  //                 (uint32_t)&stack_2[1024], TASK_FLAGS_SYSTEM);
  // ASSERT(ret != -1);

  // task_start(task_1);
  // task_start(task_2);

  // timer_init();

  // cpu_irq_start();
  // while (1) {
  //   msleep(1000);
  //   // log_printf("rINTPND = %x rINTMSK = %x rEINTMSK = %x, num = %d\n",
  //   // rINTPND, rINTMSK, rEINTMASK, num++);
  //   log_printf("first_task runing! num = %d\n", num++);
  // }

  char buff[512 * 4];
  int sector_start = 0;
  int sector_size = 4 * 64 * 4;

  nand_open();

  for (int i = 0; i < sector_size; i += 4) {
    kernel_memset(buff, 0, 512 * 4);
    int ret = nand_read(sector_start + i, buff, 4);
  }

  nand_close();

  while (1) {
  }
}