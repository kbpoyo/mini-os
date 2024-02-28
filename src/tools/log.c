#include "tools/log.h"

#include <stdarg.h>

#include "dev/uart.h"
#include "tools/klib.h"

void log_init() {
  uart_init();
  log_printf(ESC_CLEAR_SCREEN ESC_COLOR_DEFAULT);
}

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

void log_error(const char *fmt, ...) {
  char string[256];

  kernel_memset(string, 0, sizeof(string));

  va_list ap;
  va_start(ap, fmt);

  kernel_vsprintf(string, fmt, ap);
  va_end(ap);

  log_printf(ESC_COLOR_ERROR "error: %s" ESC_COLOR_DEFAULT, string);
}