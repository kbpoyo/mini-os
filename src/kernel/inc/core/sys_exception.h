#ifndef SYS_EXCEPTION_H
#define SYS_EXCEPTION_H

#include "common/types.h"

/**
 * @brief 定义mmu失效状态寄存器的错误状态码
 *
 */
// 基于段的地址变换失效，即一级描述符[1:0]位为0b00
#define MMU_ERR_FIRST_PAGE_ENTRY 0x5
// 基于页的地址变换失效，即二级描述符[1:0]位为0b00
#define MMU_ERR_SECOND_PAGE_ENTRY 0x7
// 基于页访问的地址权限不够错误
#define MMU_ERR_PAGE_ACCESS 0xf

// 定义调用门处理函数的栈帧
typedef struct _exception_frame_t {
  // 手动压入的寄存器
  uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14;

} exception_frame_t;

void undef_handler(exception_frame_t* frame);

void data_abort_handler(exception_frame_t* frame);

void prefetch_abort_handler(exception_frame_t* frame);

#endif