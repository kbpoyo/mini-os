/**
 * @file tty.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tty设备,键盘和显示器的统一抽象，键盘只读，显示器只写
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TTY_H
#define TTY_H

#include "ipc/sem.h"

// tty缓存队列
typedef struct _tty_fifo_t {
  char *buf;        // 缓冲区起始地址
  int size;         // 缓冲区大小
  int read, write;  // 缓冲区读写位置
  int count;        // 缓冲区有效数据大小，即未读数据大小
} tty_fifo_t;

#define TTY_TABLE_SIZE 3    // tty设备表的大小
#define TTY_OBUF_SIZE 512   // 输出缓存大小
#define TTY_IBUF_SIZE 512   // 输入缓存大小
#define TTY_OCRLF (1 << 0)  // 输出的换行符为"\r\n"
#define TTY_INCLR (1 << 0)  // 输入的换行符是否转换
#define TTY_IECHO (1 << 1)  // 输入的回显

// 外部程序输入的TTY控制指令宏
// 对tyy回显进行设置
#define TTY_CMD_ECHO 0x1
// 获取tty输入缓冲区的字符个数
#define TTY_CMD_IN_COUNT 0x2

// tty设备结构
typedef struct _tty_t {
  int oflags;         // 设备输出状态标志位
  int iflags;         // 设备输入状态标志位
  int console_index;  // tty对应的终端的索引

  tty_fifo_t out_fifo;  // 输出缓存队列
  tty_fifo_t in_fifo;   // 输入缓存队列

  sem_t
      out_sem;  // 输出缓冲区信号量，这东西应该配合硬件的中断程序用，单进程不需要
  sem_t in_sem;  // 输入缓冲区信号量，

  char out_buf[TTY_OBUF_SIZE];  // 输入缓存
  char in_buf[TTY_IBUF_SIZE];   // 输出缓存
} tty_t;

int tty_fifo_put(tty_fifo_t *fifo, char c);
int tty_fifo_get(tty_fifo_t *fifo, char *c);

void tty_in(char ch);
void tty_select(int tty_index);

#endif