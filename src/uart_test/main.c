
#include "tools/uart.h"


int main () {

    uart_init();

    while (1) {
        char * str = "hello world!";
        int n = 100;

        while (n--) {
            int m = 100;
            // uart_send_str("hello world!\n");
            uart_printf("str = %s, n = %d\n", str, n);
            while (m--) {
            }
            
        }
        
    }
    
}