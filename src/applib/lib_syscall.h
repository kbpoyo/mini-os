/**
 * @file lib_syscall.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 供外部应用程序编码时使用的库文件，对系统调用进行封装
 * @version 0.1
 * @date 2023-05-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include <sys/stat.h>

#include "common/os_config.h"
#include "common/types.h"
#include "core/tty.h"

#pragma pack(1)
/**
 * @brief 系统调用的参数结构体
 *
 */
typedef struct _syscall_args_t {
  int id;  // 真正的系统调用函数的id
  // 系统调用的函数的4个参数
  uint32_t arg0;
  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
} syscall_args_t;

#pragma pack()

void print_msg(const char *fmt, int arg);

// 进程相关系统调用
int getpid(void);
void msleep(int ms);
int _fork(void);
int _execve(const char *name, char *const *argv, char *const *env);
void yield(void);
int _wait(int *status);
void _exit(int status);

// 提供给newlib库的系统调用
// 文件操作相关系统调用
int _open(const char *name, int flags, ...);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);
int _close(int file);
int _lseek(int file, int offset, int dir);
int ioctl(int file, int cmd, int arg0, int arg1);
int _unlink(const char *path);

int _isatty(int file);
int _fstat(int file, struct stat *st);

// typedef long int ptrdiff_t;
char *_sbrk(ptrdiff_t incr);

int dup(int file);
// 文件目录项结构
typedef struct dirent {
  int index;
  int type;
  char name[14];
  int size;
} dirent;

// 文件目录对象结构
typedef struct _DIR {
  int index;
  struct dirent dirent;
} DIR;

// 目录操作的系统调用
DIR *opendir(const char *path);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

// 查看系统情况
int memory_use_stat(char *buf, int size);
int task_use_stat(char *buf, int size, int *task_count);

#endif