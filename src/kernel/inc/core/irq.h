#ifndef IRQ_H
#define IRQ_H

#include "common/register_addr.h"

//定义中断向量号

//无次级中断
#define NOSUBINT    -1

//看门狗中断
#define INT_WDT_PRIM    9
#define INT_WDT_SECON   13

//串口中断
#define INT_UART0   28
#define INT_TXD0_PRIM   INT_UART0
#define INT_RXD0_PRIM   INT_UART0
#define INT_UART0_ERR_PRIM  INT_UART0
#define INT_RXD0_SECON  0
#define INT_TXD0_SECON  1
#define INT_UART0_ERR_SECON 2

//定时器中断
#define INT_TIMER0  10
#define INT_TIMER1  11
#define INT_TIMER2  12
#define INT_TIMER3  13
#define INT_TIMER4  14

//外部中断
#define EINT8_23    5


void irq_init();
void irq_start();
void irq_close();
void irq_enable(int irq_num_prim, int irq_num_secon);
void irq_disable(int irq_num_prim, int irq_num_secon);
void irq_enable_all();
void irq_disable_all();
void irq_clear(int irq_num_prim, int irq_num_secon);
void irq_clear_all();

void irq_handler();


#endif