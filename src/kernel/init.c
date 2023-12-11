
#include "tools/uart.h"


int kernel_init () {

    uart_init();

    while (1) {
        char * str = "hello world!";
        int n = 100;

        while (n--) {
            int m = 100;
            // uart_send_str("hello world!\n");
            while (m--) {
                uart_printf("str = %s, n = %d, m = %d\n", str, n, m);
            }
            
        }
        
    }
    
}