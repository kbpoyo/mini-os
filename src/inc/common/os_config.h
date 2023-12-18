#ifndef OS_CONFIG_H
#define OS_CONFIG_H



//时钟配置
#define OS_FCLK (400000000) //hz
#define OS_HCLK (OS_FCLK / 4)
#define OS_PCLK (OS_HCLK / 2)

//内核加载地址
#define KERNEL_ADDR    0x30000000

//异常处理函数的索引号,用于在内核的异常向量处理接口中索引异常处理方法
#define EXCEPTION_RESET 0
#define EXCEPTION_UNDEF 1
#define EXCEPTION_SWI   2
#define EXCEPTION_PREFETCH_ABORT 3
#define EXCEPTION_DATA_ABORT 4
#define EXCEPTION_IRQ   5
#define EXCEPTION_FIQ   6

//内核堆栈配置
#define STACK_SVC_SIZE  0x400
#define STACK_ADDR_FIQ  0x34000000
#define STACK_ADDR_IRQ              STACK_ADDR_FIQ - STACK_SVC_SIZE
#define STACK_ADDR_ABORT            STACK_ADDR_IRQ - STACK_SVC_SIZE
#define STACK_ADDR_SVC              STACK_ADDR_ABORT - STACK_SVC_SIZE
#define STACK_ADDR_UND              STACK_ADDR_SVC - STACK_SVC_SIZE
#define STACK_ADDR_SYS_AND_USER     STACK_ADDR_UND - STACK_SVC_SIZE



//定义任务时间片长度
#define TASK_TIME_SLICE_MS 1000 //ms

#endif
