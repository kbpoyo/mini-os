#ifndef TIMER_H
#define TIMER_H

#include "common/register_addr.h"

#define rTCFG0_INIT ((250 - 1) << 8)    //设置定时器4的预分频为250
#define rTCGG1_INIT (1 << 16)   //设置定时器4的分频通道为1/4

//定时器4的输入频率为PCLK/(250*4) = 50Mhz/1000 = 5e4 hz
//即定时器的分辨率为2e-5s = 20us
//定时器计数器为16位，即最大值为0xffff, 所以定时器支持最大定时时间为1,310,700us = 1.3107s
#define TIMER_RESOLVING_POWER   20 //us


#define HAND_REFLASH_4 (1 << 21)
#define AUTORELOAD_AND_START_4    ((1 << 22) | (1 << 20))

void timer_init();


#endif