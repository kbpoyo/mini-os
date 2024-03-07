/**
 * @file task.h
 * @author kbpoyo (kbpoyo.com)
 * @brief
 * @version 0.1
 * @date 2023-01-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TASK_H
#define TASK_H

#include "common/types.h"
#include "fs/file.h"
#include "tools/list.h"

// 定义任务名称缓冲区大小
#define TASK_NAME_SIZE 32

// 静态分配任务，定义任务数量
#define TASK_COUNT 128

// 定义每个进程所能拥有的时间切片数量
#define TASK_TIME_SLICE_DEFAULT 10

// 定义空闲进程的栈空间大小
#define EMPTY_TASK_STACK_SIZE 128

// 定义进程可打开的文件数量大小
#define TASK_OFILE_SIZE 128

// 设置任务进程的特权级标志位
#define TASK_FLAGS_SYSTEM (1 << 0)  // 内核特权级即最高特权级
#define TASK_FLAGS_USER (0 << 0)    // 用户特权级

// 设置用户初始状态寄存器
#define TASK_CPSR_USER 0x10
#define TASK_CPSR_SYS 0x1f

// 定义任务状态枚举类型
typedef enum _task_state_t {
  TASK_CREATED,   // 已创建，任务被创建，但为加入就绪队列
  TASK_RUNNING,   // 运行态，正在运行
  TASK_SLEEP,     // 延时态，等待其线程唤醒或超时唤醒
  TASK_READY,     // 就绪态，可运行
  TASK_WAITTING,  // 等待态，只能等待其他线程唤醒
  TASK_BLOCKED,   // 阻塞态，等待外部资源或锁准备好
  TASK_ZOMBIE,  // 僵尸态，进程已死掉，等待资源被父进程回收
  TASK_ORPHAN,  // 孤儿态，进程已死掉，并且其父进程提前死掉
} task_state_t;

#pragma pack(1)
typedef struct _task_switch_t {
  uint32_t svc_sp;  // 记录内核栈顶位置
  uint32_t page_dir;

} task_switch_t;

typedef struct _register_group_t {
  uint32_t spsr, cpsr;
  uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
} register_group_t;
#pragma pack()

// 定义可执行任务的数据结构,即PCB进程控制块，书p406
typedef struct _task_t {
  task_state_t state;      // 任务状态
  struct _task_t *parent;  // 父进程控制块的地址
  int pid;                 // 进程id
  int status;              // 进程退出的状态值

  int slice_max;   // 任务所能拥有的最大时间分片数
  int slice_curr;  // 任务当前的所拥有的时间分片数
  int sleep;       // 当前任务延时的时间片数

  uint32_t heap_start;  // 堆起始地址
  uint32_t heap_end;    // 堆结束地址

  char name[TASK_NAME_SIZE];  // 任务名称

  list_node_t
      ready_node;  // 用于插入就绪队列或休眠队列的节点，标记task在ready_list或sleep_list中的位置
  list_node_t task_node;  // 用于插入任务队列的节点，标记task在任务队列中的位置
  list_node_t
      wait_node;  // 用于插入信号量对象的等待队列的节点，标记task正在等待信号量

  task_switch_t task_sw;  // 存放内核栈指针和任务页目录表，随着进程切换而切换
  uint32_t svc_sp_top;  // 记录内核栈的起始位置

  register_group_t reg_group;           // 任务寄存器组
  file_t *file_table[TASK_OFILE_SIZE];  // 任务进程所拥有的文件表

} task_t;

typedef uint32_t cpu_state_t;

int task_init(task_t *task, const char *name, uint32_t entry, uint32_t esp,
              uint32_t flag);
// void task_switch_from_to(task_t *from, task_t *to);

// 定义任务管理器
typedef struct _task_manager_t {
  task_t *curr_task;  // 当前正在执行的任务

  list_t ready_list;  // 就绪队列，包含所有已准备好的可执行任务
  list_t task_list;   // 任务队列，包含所有的任务
  list_t sleep_list;  // 延时队列，包含当前需要延时的任务

  task_t first_task;  // 执行的第一个任务
  task_t
      empty_task;  // 一个空的空闲进程，当所有进程都延时运行时，让cpu运行空闲进程

} task_manager_t;

// 定义任务入口参数的数据结构
typedef struct _task_args_t {
  uint32_t argc;      // 入口参数个数
  char *const *argv;  // 入口参数的字符串数组
} task_args_t;

void task_manager_init(void);
uint32_t task_enter_protection(void);
void task_leave_protection(uint32_t state);
void task_first_init(void);
task_t *task_first_task(void);
void task_set_ready(task_t *task);
void task_set_unready(task_t *task);
void task_set_sleep(task_t *task, uint32_t slice);
void task_set_wakeup(task_t *task);
void task_slice_end(void);
void task_switch(void);
task_t *task_current(void);
task_t *task_alloc(void);

void task_start(task_t *task);

// //系统调用函数
void sys_sleep(uint32_t ms);
void sys_yield(void);
int sys_getpid(void);
int sys_fork(void);
int sys_execve(char *name, char *const *argv, char *const *env);
void sys_exit(int status);
int sys_wait(int *status);
int sys_task_stat(char *buf, int size, int *task_count);

#endif