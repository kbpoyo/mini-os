
#include "soc/uart.h"
#include "soc/gpio.h"
#include "core/irq.h"

int kernel_init () {

    gpio_init();

    uart_init();

    irq_init();


    while (1) {
        int n = 100;

        while (n--) {
            int m = 100;
            // uart_send_str("hello world!\n");
            while (m--) {
                uart_printf("rINTPND = %x rINTMSK = %x\n", rINTPND, rINTMSK);
            }
            
        }
        
    }
    
}