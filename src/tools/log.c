#include "tools/log.h"

#include <stdarg.h>

#include "core/dev.h"
#include "dev/uart.h"
#include "tools/klib.h"

// 定义互斥锁保护输出资源，确保输出为原子操作
static mutex_t mutex;

// 日志打印需要的设备id
static int log_dev_id;

/**
 * @brief  初始化串行端口寄存器COM1
 *
 */
void log_init(void) {
  // 初始化互斥锁
  mutex_init(&mutex);

  // 打开一个tty设备用于日志打印
  log_dev_id = dev_open(DEV_TTY, 0, (void *)0);
}

/**
 * @brief  格式化输出到串口
 *
 * @param formate
 * @param ...
 */
void log_printf(const char *formate, ...) {
  // 1.设置字符缓冲区
  char str_buf[128];
  kernel_memset(str_buf, '\0', sizeof(str_buf));

  // 2.获取可变参数并将其格式化到缓冲区中
  va_list args;
  va_start(args, formate);
  kernel_vsprintf(str_buf, formate, args);
  va_end(args);

  mutex_lock(&mutex);  // TODO:加锁

  // tty设备在显示器上写入时是根据当前光标位置来的，所以不需要传入addr参数
  dev_write(log_dev_id, 0, str_buf, kernel_strlen(str_buf));

  mutex_unlock(&mutex);  // TODO:解锁
}

/**
 * @brief 打印错误日志信息
 *
 * @param fmt
 * @param ...
 */
void log_error(const char *fmt, ...) {
  char string[256];

  kernel_memset(string, 0, sizeof(string));

  va_list ap;
  va_start(ap, fmt);

  kernel_vsprintf(string, fmt, ap);
  va_end(ap);

  log_printf(ESC_COLOR_ERROR "error: %s" ESC_COLOR_DEFAULT "\n", string);
}