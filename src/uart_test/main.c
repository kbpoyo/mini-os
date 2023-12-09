
#include "tools/uart.h"


int main () {

    uart_init();

    while (1) {
        char * str = "hello world!";
        int n = 100;
        uart_printf("str = %s, n = %d\n", n);
        // uart_send_str("hello wold\n");
        while (n--) {
            int m = 100;
            while (m--) {
            }
            
        }
        
    }
    
}