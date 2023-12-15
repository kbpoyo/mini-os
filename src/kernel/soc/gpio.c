#include "soc/gpio.h"


/**
 * @brief 初始化使用到的引脚
 * 
 */
void gpio_init() {

    //将key1按键初始化为EINT8中断
    rGPGCON = rGPGCON_INIT;


    //设置将三个串口的TXD和RXD引脚
    rGPHCON = rGPHCON_INIT;


    //将外部中断全设置为下降沿触发
    rEXTINT0 = rEXTINT_INIT;
    rEXTINT1 = rEXTINT_INIT;
    rEXTINT2 = rEXTINT_INIT;


}