
#include "core/irq.h"
#include "soc/gpio.h"
#include "soc/uart.h"
#include "soc/timer.h"
#include "common/cpu_instr.h"
#include "core/task.h"
void delay(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (uint32_t j = 0; j < 400; j++)
            ;
    }
}

int kernel_init() {
    gpio_init();

    uart_init();

    irq_init();

    cpu_irq_start();

    // timer_init();

    while (1) {
        delay(1000);
        uart_printf("rINTPND = %x rINTMSK = %x rEINTMSK = %x\n", rINTPND, rINTMSK, rEINTMASK);
    }
}