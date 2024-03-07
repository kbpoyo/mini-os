#ifndef LOG_H
#define LOG_H

// 定义ESC序列生成宏
#define ESC_CMD2(Pn, cmd) "\x1b[" #Pn #cmd  //'#'用来将数字解析为字符串
#define ESC_CLEAR_SCREEN ESC_CMD2(2, J)     // 清屏序列
#define ESC_MOVE_CURSOR(row, col) "\x1b[" #row ";" #col "H"  // 移动光标序列
#define ESC_COLOR_ERROR ESC_CMD2(31, m)   // 更换前景色为红色
#define ESC_COLOR_DEFAULT ESC_CMD2(0, m)  // 更换前景色为默认白色



void log_init();
void log_printf(const char *fmt, ...);
void log_error(const char *fmt, ...);
#endif