#ifndef UPART_H
#define UPART_H

#include "common/types.h"

// UART
#define rULCON0 (*(volatile unsigned *)0x50000000)    // UART 0 Line control
#define rUCON0 (*(volatile unsigned *)0x50000004)     // UART 0 Control
#define rUFCON0 (*(volatile unsigned *)0x50000008)    // UART 0 FIFO control
#define rUMCON0 (*(volatile unsigned *)0x5000000c)    // UART 0 Modem control
#define rUTRSTAT0 (*(volatile unsigned *)0x50000010)  // UART 0 Tx/Rx status
#define rUERSTAT0 (*(volatile unsigned *)0x50000014)  // UART 0 Rx error status
#define rUFSTAT0 (*(volatile unsigned *)0x50000018)   // UART 0 FIFO status
#define rUMSTAT0 (*(volatile unsigned *)0x5000001c)   // UART 0 Modem status
#define rUBRDIV0 (*(volatile unsigned *)0x50000028)   // UART 0 Baud rate divisor

#define rULCON1 (*(volatile unsigned *)0x50004000)    // UART 1 Line control
#define rUCON1 (*(volatile unsigned *)0x50004004)     // UART 1 Control
#define rUFCON1 (*(volatile unsigned *)0x50004008)    // UART 1 FIFO control
#define rUMCON1 (*(volatile unsigned *)0x5000400c)    // UART 1 Modem control
#define rUTRSTAT1 (*(volatile unsigned *)0x50004010)  // UART 1 Tx/Rx status
#define rUERSTAT1 (*(volatile unsigned *)0x50004014)  // UART 1 Rx error status
#define rUFSTAT1 (*(volatile unsigned *)0x50004018)   // UART 1 FIFO status
#define rUMSTAT1 (*(volatile unsigned *)0x5000401c)   // UART 1 Modem status
#define rUBRDIV1 (*(volatile unsigned *)0x50004028)   // UART 1 Baud rate divisor

#define rULCON2 (*(volatile unsigned *)0x50008000)    // UART 2 Line control
#define rUCON2 (*(volatile unsigned *)0x50008004)     // UART 2 Control
#define rUFCON2 (*(volatile unsigned *)0x50008008)    // UART 2 FIFO control
#define rUMCON2 (*(volatile unsigned *)0x5000800c)    // UART 2 Modem control
#define rUTRSTAT2 (*(volatile unsigned *)0x50008010)  // UART 2 Tx/Rx status
#define rUERSTAT2 (*(volatile unsigned *)0x50008014)  // UART 2 Rx error status
#define rUFSTAT2 (*(volatile unsigned *)0x50008018)   // UART 2 FIFO status
#define rUMSTAT2 (*(volatile unsigned *)0x5000801c)   // UART 2 Modem status
#define rUBRDIV2 (*(volatile unsigned *)0x50008028)   // UART 2 Baud rate divisor

#ifdef __BIG_ENDIAN
#define rUTXH0 (*(volatile unsigned char *)0x50000023)  // UART 0 Transmission Hold
#define rURXH0 (*(volatile unsigned char *)0x50000027)  // UART 0 Receive buffer
#define rUTXH1 (*(volatile unsigned char *)0x50004023)  // UART 1 Transmission Hold
#define rURXH1 (*(volatile unsigned char *)0x50004027)  // UART 1 Receive buffer
#define rUTXH2 (*(volatile unsigned char *)0x50008023)  // UART 2 Transmission Hold
#define rURXH2 (*(volatile unsigned char *)0x50008027)  // UART 2 Receive buffer

#define WrUTXH0(ch) (*(volatile unsigned char *)0x50000023) = (unsigned char)(ch)
#define RdURXH0() (*(volatile unsigned char *)0x50000027)
#define WrUTXH1(ch) (*(volatile unsigned char *)0x50004023) = (unsigned char)(ch)
#define RdURXH1() (*(volatile unsigned char *)0x50004027)
#define WrUTXH2(ch) (*(volatile unsigned char *)0x50008023) = (unsigned char)(ch)
#define RdURXH2() (*(volatile unsigned char *)0x50008027)

#define UTXH0 (0x50000020 + 3)  // Byte_access address by DMA
#define URXH0 (0x50000024 + 3)
#define UTXH1 (0x50004020 + 3)
#define URXH1 (0x50004024 + 3)
#define UTXH2 (0x50008020 + 3)
#define URXH2 (0x50008024 + 3)

#else                                                   // Little Endian
#define rUTXH0 (*(volatile unsigned char *)0x50000020)  // UART 0 Transmission Hold
#define rURXH0 (*(volatile unsigned char *)0x50000024)  // UART 0 Receive buffer
#define rUTXH1 (*(volatile unsigned char *)0x50004020)  // UART 1 Transmission Hold
#define rURXH1 (*(volatile unsigned char *)0x50004024)  // UART 1 Receive buffer
#define rUTXH2 (*(volatile unsigned char *)0x50008020)  // UART 2 Transmission Hold
#define rURXH2 (*(volatile unsigned char *)0x50008024)  // UART 2 Receive buffer

#define WrUTXH0(ch) (*(volatile unsigned char *)0x50000020) = (unsigned char)(ch)
#define RdURXH0() (*(volatile unsigned char *)0x50000024)
#define WrUTXH1(ch) (*(volatile unsigned char *)0x50004020) = (unsigned char)(ch)
#define RdURXH1() (*(volatile unsigned char *)0x50004024)
#define WrUTXH2(ch) (*(volatile unsigned char *)0x50008020) = (unsigned char)(ch)
#define RdURXH2() (*(volatile unsigned char *)0x50008024)

#define UTXH0 (0x50000020)  // Byte_access address by DMA
#define URXH0 (0x50000024)
#define UTXH1 (0x50004020)
#define URXH1 (0x50004024)
#define UTXH2 (0x50008020)
#define URXH2 (0x50008024)

#endif

#define PCLK 50000000    // APB时钟
#define BAUDRATE 115200  // 波特率

// 接收发送状态寄存器的设置
#define STATE_REC_BUFF_ISREADY (0x1 << 0)  // 接收缓存是否准备好
#define STATE_TRA_BUFF_ISEMPTY (0x1 << 1)       // 放送缓存是否为空

// 错误状态寄存器设置
#define ERR_BREAK_DETECT (0x1 << 3)  // 自动置 1 来指出一个终止信号已发出
#define ERR_FRAME (0x1 << 2)         // 只要在接收操作中帧错误出现则自动置 1
#define ERR_PARITY (0x1 << 1)        // 只要在接收操作中出现奇偶校验错误则自动置 1
#define ERR_OVER (0x1 << 0)          // 只要在接收过程中出现溢出错误则自动置 1

void uart_init();
void uart_send_byte(uint8_t data);
void uart_send_str(const char *str);
void uart_printf(char *fmt,...);


#endif