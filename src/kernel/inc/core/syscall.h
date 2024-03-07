/**
 * @file syscall.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 定义系统调用相关参数和数据结构
 * @version 0.1
 * @date 2023-05-09
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "common/types.h"

// 定义调用门描述符中，参数个数属性，即系统调用函数的参数个数
#define SYSCALL_PARAM_COUNT 5

// 定义进程相关系统调用的id
#define SYS_sleep 0   // 延时函数
#define SYS_getpid 1  // 获取pid
#define SYS_fork 2    // fork子进程
#define SYS_execve 3  // 加载外部程序
#define SYS_yield 4   // 进程主动放弃cpu
#define SYS_exit 5    // 进程主动退出
#define SYS_wait 6    // 回收进程资源

// 文件相关系统调用
#define SYS_open 50
#define SYS_read 51
#define SYS_write 52
#define SYS_close 53
#define SYS_lseek 54
#define SYS_isatty 55
#define SYS_fstat 56
#define SYS_dup 57
#define SYS_ioctl 58
#define SYS_unlink 59

// 目录分配系统调用
#define SYS_opendir 60
#define SYS_readdir 61
#define SYS_closedir 62

// 内存分配系统调用
#define SYS_sbrk 63

#define SYS_printmsg 10  // 临时使用的打印函数

#define SYS_memory_stat 64
#define SYS_task_stat 65

#pragma pack(1)
/**
 * @brief 系统调用的参数结构体
 *
 */
typedef struct _syscall_args_kernel_t {
  int id;  // 真正的系统调用函数的id
  // 系统调用的函数的4个参数
  uint32_t arg0;
  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
} syscall_args_kernel_t;

// 定义调用门处理函数的栈帧
typedef struct _syscall_frame_t {
  // 手动压入的寄存器
  uint32_t spsr;
  syscall_args_kernel_t* syscall_args;  //(用r0进行传递)返回值通过r0进行接收
  uint32_t r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
  uint32_t r11, r12, sp, r14, pc;

} syscall_frame_t;

#pragma pack()

typedef int (*sys_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2,
                             uint32_t arg3);

void exception_handler_syscall(void);

#endif