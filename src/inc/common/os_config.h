#ifndef OS_CONFIG_H
#define OS_CONFIG_H



//时钟配置
#define OS_FCLK (400000000) //hz
#define OS_HCLK (OS_FCLK >> 2)
#define OS_PCLK (OS_HCLK >> 1)

//内核加载地址
#define KERNEL_INIT_ADDR    0x30000000

#endif
