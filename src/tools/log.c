#include <stdarg.h>
#include "tools/log.h"
#include "tools/klib.h"
#include "soc/uart.h"

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
    va_start(ap,fmt);

    kernel_vsprintf(string,fmt,ap);
    va_end(ap);


    uart_send_str(string);
}
