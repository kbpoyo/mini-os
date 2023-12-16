#ifndef GPIO_H
#define GPIO_H

#include "common/register_addr.h"

//将key1按键初始化为EINT8中断
#define rGPGCON_INIT    ((0x2 << 6) | 0x2)   
//将外部中断全设置为下降沿触发
#define rEXTINT_INIT    0x22222222  

//设置将三个串口的TXD和RXD引脚
#define rGPHCON_INIT    ((0x2 << 14) | (0x2 << 12) | (0x2 << 10) | (0x2 << 8) | (0x2 << 6) | (0x2 << 4))


void gpio_init();

#endif