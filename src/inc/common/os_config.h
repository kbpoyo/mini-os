#ifndef OS_CONFIG_H
#define OS_CONFIG_H

// 处理器模式
#define CPU_MODE_USER 0x10
#define CPU_MODE_FIQ 0x11
#define CPU_MODE_IRQ 0x12
#define CPU_MODE_SVC 0x13
#define CPU_MODE_ABT 0x17
#define CPU_MODE_UND 0x1b
#define CPU_MODE_SYS 0x1f

// 处理器中断屏蔽位
#define CPU_MASK_IRQ (0x1 << 7)
#define CPU_MASK_FIQ (0x1 << 6)

// 时钟配置
#define OS_FCLK (400000000)  // hz
#define OS_HCLK (OS_FCLK / 4)
#define OS_PCLK (OS_HCLK / 2)

// SDRAM起始地址与大小
#define SDRAM_START 0x30000000
#define SDRAM_SIZE (64 * 1024 * 1024)
// 内部sdram的起始地址和大小
#define SDRAM_INSIDE_START 0x0
#define SDRAM_INSIDE_SIZE (4 * 1024)

// 内核加载地址
#define KERNEL_ADDR SDRAM_START

// 异常处理函数的索引号,用于在内核的异常向量处理接口中索引异常处理方法
#define EXCEPTION_RESET 0
#define EXCEPTION_UNDEF 1
#define EXCEPTION_SWI 2
#define EXCEPTION_PREFETCH_ABORT 3
#define EXCEPTION_DATA_ABORT 4
#define EXCEPTION_IRQ 5
#define EXCEPTION_FIQ 6

// 内核堆栈配置
#define STACK_SVC_SIZE 0x4000
#define STACK_ADDR_SVC 0x30100000
#define STACK_ADDR_FIQ STACK_ADDR_SVC
#define STACK_ADDR_IRQ STACK_ADDR_SVC
#define STACK_ADDR_ABORT STACK_ADDR_SVC
#define STACK_ADDR_UND STACK_ADDR_SVC
#define STACK_ADDR_SYS_AND_USER (STACK_ADDR_SVC - STACK_SVC_SIZE)

// 定义任务时间片长度
#define TASK_TIME_SLICE_MS 10  // ms，最大支持1.3107s

// 定义操作系统版本
#define OS_VERSION "1.0.0"

// disk类型设备的0xa1分区作为系统的根目录分区
#define ROOT_DEV DEV_DISK, 0xa1

#endif
