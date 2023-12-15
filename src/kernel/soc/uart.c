#include <stdarg.h>
#include "common/types.h"
#include "soc/uart.h"
#include "tools/klib.h"

static int which_uart __attribute__((section(".data"))) = 0; 

/**串口初始化*/
void uart_init() {
    rUFCON0 = 0x0;  // UART channel 0 FIFO control register, FIFO disable
    rUFCON1 = 0x0;  // UART channel 1 FIFO control register, FIFO disable
    rUFCON2 = 0x0;  // UART channel 2 FIFO control register, FIFO disable
    rUMCON0 = 0x0;  // UART chaneel 0 MODEM control register, AFC disable
    rUMCON1 = 0x0;  // UART chaneel 1 MODEM control register, AFC disable

    // UART0
    rULCON0 = 0x3;                                        // Line control register : Normal,No parity,1 stop,8 bits
                                                          //     [10]       [9]     [8]        [7]        [6]      [5]         [4]           [3:2]        [1:0]
                                                          //  Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
                                                          //      0          1       0    ,     0          1        0           0     ,       01          01
                                                          //    PCLK       Level    Pulse    Disable    Generate  Normal      Normal        Interrupt or Polling
    rUCON0 = 0x245;                                       // Control register
    rUBRDIV0 = ((int)(OS_PCLK / (16 * BAUDRATE)) - 1);  // Baud rate divisior register 0
    // UART1
    rULCON1 = 0x3;
    rUCON1 = 0x245;
    rUBRDIV1 = ((int)(OS_PCLK / (16 * BAUDRATE)) - 1);
    // UART2
    rULCON2 = 0x3;
    rUCON2 = 0x245;
    rUBRDIV2 = ((int)(OS_PCLK / (16 * BAUDRATE)) - 1);

    //for (int i = 0; i < 100; i++);

    char * str = "\nuart init success!\n";
    uart_send_str(str);
}

/**
 * @brief 串口发送字节
 * 
 * @param data 
 */
void uart_send_byte(uint8_t data) {
    if (which_uart == 0) {
        if (data == '\n') {
            while (!(rUTRSTAT0 & STATE_TRA_BUFF_ISEMPTY));
            WrUTXH0('\r');
        }

        while (!(rUTRSTAT0 & STATE_TRA_BUFF_ISEMPTY));
        WrUTXH0(data);

    } else if (which_uart == 1) {
        if (data == '\n') {
            while (!(rUTRSTAT1 & STATE_TRA_BUFF_ISEMPTY))    ;
            WrUTXH1('\r');
        }

        while (!(rUTRSTAT1 & STATE_TRA_BUFF_ISEMPTY));
        WrUTXH1(data);

    } else if (which_uart == 2) {
        if (data == '\n') {
            while (!(rUTRSTAT2 & STATE_TRA_BUFF_ISEMPTY))    ;
            WrUTXH2('\r');
        }

        while (!(rUTRSTAT2 & STATE_TRA_BUFF_ISEMPTY));
        WrUTXH2(data);
    }
}

/**
 * @brief 串口发送字符串
 * 
 * @param str 
 */
void uart_send_str(const char *str) {
    const char * temp = str;
    while(*temp) {
        uart_send_byte(*(temp++));
    }
}

/**
 * @brief 串口格式化输出
 * 
 * @param fmt 
 * @param ... 
 */
void uart_printf(char *fmt, ...) {
    char string[256];

    kernel_memset(string, 0, sizeof(string));
    
    va_list ap;
    va_start(ap,fmt);

    kernel_vsprintf(string,fmt,ap);
    va_end(ap);


    uart_send_str(string);
}
