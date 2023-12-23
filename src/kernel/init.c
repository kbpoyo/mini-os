
#include "core/irq.h"
#include "soc/gpio.h"
#include "soc/uart.h"
#include "soc/timer.h"
#include "common/cpu_instr.h"
#include "core/task.h"
#include "tools/log.h"
#include "applib/lib_syscall.h"
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
        
        log_printf("task_1 runing!, num = %d pid = 0x%x\n", num++, getpid());
        msleep(1);
    }
    
}

uint32_t stack_2[1024] __attribute__((section(".data"), aligned(16))) = {0};

void task_test_2(void) {
     while (1) {
        
        log_printf("task_2 runing! num = %d pid = 0x%x\n", num++, getpid());
        msleep(1);
    }
}


int kernel_init() {
    gpio_init();

    uart_init();

    irq_init();

    cpu_irq_start();

    task_manager_init();

    task_first_init();

    task_t * task_1 = task_alloc();
    int ret = task_init(task_1, "task_1", (uint32_t)task_test_1, (uint32_t)&stack_1[1024], TASK_FLAGS_SYSTEM);
    ASSERT(ret != -1);

    task_t * task_2 = task_alloc();
    ret = task_init(task_2, "task_2", (uint32_t)task_test_2, (uint32_t)&stack_2[1024], TASK_FLAGS_SYSTEM);
    ASSERT(ret != -1);

    task_start(task_1);
    task_start(task_2);

    timer_init();

    

    while (1) {
        msleep(1);
        log_printf("rINTPND = %x rINTMSK = %x rEINTMSK = %x, num = %d pid = 0x%x\n", rINTPND, rINTMSK, rEINTMASK, num++, getpid());
    }
}