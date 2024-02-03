#ifndef UPART_H
#define UPART_H

#include "common/types.h"
#include "common/register_addr.h"
#include "common/os_config.h"



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