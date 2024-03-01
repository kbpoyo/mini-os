#ifndef MEMORY_H
#define MEMORY_H

#include "common/os_config.h"
#include "core/mmu.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"

// 定义任务内核栈分配页数
#define MEM_TASK_STACK_PAGE_COUNT 4

// 低1mb内存空间
#define MEM_EXT_START (KERNEL_ADDR + 1024 * 1024)
// 真正给操作系统的物理内存空间的结束地址, mini2440只有64mb
#define MEM_EXT_END (SDRAM_START + SDRAM_SIZE)
// 虚拟空间中，用户进程的起始地址设置为 0x8000 0000,
// 以下的空间映射给操作系统使用,即2GB
#define MEM_TASK_BASE 0x80000000
// 定义应用程序的栈空间起始地址的虚拟地址,即给每个进程分配了1.5gb的虚拟空间大小
#define MEM_TASK_STACK_TOP (0xE0000000)
// 定义每个应用程序的栈空间大小为50页
#define MEM_TASK_STACK_SIZE (MEM_PAGE_SIZE * 50)
// 定义分配给每个应用程序的入口参数的空间大小
#define MEM_TASK_ARG_SIZE (MEM_PAGE_SIZE * 4)

// 内存分配对象
typedef struct _addr_alloc_t {
  mutex_t mutex;       // 分配内存时进行临界资源管理
  bitmap_t bitmap;     // 管理内存页的位图结构
  uint32_t start;      // 管理内存区域的起始地址
  uint32_t size;       // 内存区域的大小
  uint32_t page_size;  // 页的大小
  uint8_t page_ref
      [(MEM_EXT_END - MEM_EXT_START) /
       MEM_PAGE_SIZE];  // TODO,由于引用计数占用空间较大，经计算，在1mb一下加载内核，管理内存大小不能超过1gb

} addr_alloc_t;

// 定义内存映射的数据结构
typedef struct _memory_map_t {
  void *vstart;           // 虚拟地址空间的起始地址
  void *vend;             // 虚拟地址空间的结束地址
  void *pstart;           // 物理地址空间的起始地址
  uint32_t access_perim;  // 该映射段的特权级，用户或者内核
} memory_map_t;

// 串口相关寄存器地址范围
#define MEM_UART_START 0x50000000
#define MEM_UART_END 0x50009000

// 中断相关寄存器地址范围映射
#define MEM_IRQ_START 0x4a000000
#define MEM_IRQ_END 0x4a00001c

// 定时器相关寄存器范围
#define MEM_TIMER_START 0x51000000
#define MEM_TIMER_END 0x51000040

// GPIO相关寄存器组
#define MEM_GPIO_START 0x56000000
#define MEM_GPIO_END 0x560000bc

// nandflash相关寄存器组
#define MEM_NADNFLASH_START 0x4E000000
#define MEM_NANDFLASH_END 0x4E00003c

void memory_init();
uint32_t memory_creat_uvm(void);
// int memory_copy_uvm(uint32_t to_page_dir, uint32_t from_page_dir);
// void memory_destroy_uvm(uint32_t page_dir);
// int memory_alloc_for_page_dir(uint32_t page_dir, uint32_t vaddr, uint32_t
// alloc_size, uint32_t privilege); uint32_t memory_get_paddr(uint32_t page_dir,
// uint32_t vaddr);

int memory_alloc_page_for(uint32_t vaddr, uint32_t alloc_size,
                          uint32_t priority);

uint32_t memory_alloc_page(int page_count);
uint32_t memory_alloc_page_align(int page_count, int align);

void memory_free_page(uint32_t addr);
// int memory_copy_uvm_data(uint32_t to_vaddr, uint32_t to_page_dir,
// uint32_t from_vaddr, uint32_t size);

// char *sys_sbrk(int incr);

void memory_show_bitmap();

#endif