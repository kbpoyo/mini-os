#include "tools/log.h"

#include <stdarg.h>

#include "dev/uart.h"
#include "tools/klib.h"

/**
 * @brief 日志输出
 *
 * @param fmt
 * @param ...
 */
void log_printf(const char *fmt, ...) {
  char string[256];

  kernel_memset(string, 0, sizeof(string));

  va_list ap;
  va_start(ap, fmt);

  kernel_vsprintf(string, fmt, ap);
  va_end(ap);

  uart_send_str(string);
}
