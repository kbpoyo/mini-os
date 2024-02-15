#ifndef SYS_EXCEPTION_H
#define SYS_EXCEPTION_H

#include "common/types.h"

// 定义调用门处理函数的栈帧
typedef struct _exception_frame_t {
  // 手动压入的寄存器
  uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14;

} exception_frame_t;

void undef_handler(exception_frame_t* frame);

void data_abort_handler(exception_frame_t* frame);

void prefetch_abort_handler(exception_frame_t* frame);

#endif