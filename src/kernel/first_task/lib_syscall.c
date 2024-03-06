#include "applib/lib_syscall.h"

#include <stdlib.h>

#include "common/os_config.h"
#include "common/types.h"
#include "core/syscall.h"

int sys_call(syscall_args_t *args) {
  int ret;

  __asm__ __volatile__(
      "swi #0x80\n\t"
      "mov %[ret], r0\n\t"
      : [ret] "=r"(ret)
      :
      : "r0"

  );

  return ret;
}

/**
 * @brief 以ms为单位进行延时
 *
 * @param ms
 */
void msleep(int ms) {
  if (ms <= 0) return;

  syscall_args_t args;
  args.id = SYS_sleep;
  args.arg0 = ms;

  sys_call(&args);
}

/**
 * @brief 获取用户进程id
 *
 * @return int
 */
int getpid(void) {
  syscall_args_t args;
  args.id = SYS_getpid;

  return sys_call(&args);
}

void print_msg(const char *fmt, int arg) {
  syscall_args_t args;
  args.id = SYS_printmsg;
  args.arg0 = (uint32_t)fmt;
  args.arg1 = arg;

  sys_call(&args);
}

int fork(void) {
  syscall_args_t args;
  args.id = SYS_fork;

  return sys_call(&args);
}

/**
 * @brief 加载执行外部程序
 *
 * @param name 外部程序名
 * @param argv
 * 外部程序的参数，字符串常量指针，即字符串数组，数组中的char*值为常量
 * @param env  所加载程序的环境变量
 * @return int
 */
int execve(const char *name, char *const *argv, char *const *env) {
  syscall_args_t args;
  args.id = SYS_execve;
  args.arg0 = (uint32_t)name;
  args.arg1 = (uint32_t)argv;
  args.arg2 = (uint32_t)env;

  return sys_call(&args);
}

/**
 * @brief 进程主动放弃cpu
 *
 */
void yield(void) {
  syscall_args_t args;
  args.id = SYS_yield;

  sys_call(&args);
}

/**
 * @brief 打开文件
 *
 * @param name
 * @param flags
 * @param ...
 * @return int
 */
int _open(const char *name, int flags, ...) {
  syscall_args_t args;
  args.id = SYS_open;
  args.arg0 = (uint32_t)name;
  args.arg1 = flags;

  return sys_call(&args);
}

/**
 * @brief 读取文件
 *
 * @param file
 * @param ptr
 * @param len
 * @return int
 */
int _read(int file, char *ptr, int len) {
  syscall_args_t args;
  args.id = SYS_read;
  args.arg0 = file;
  args.arg1 = (uint32_t)ptr;
  args.arg2 = len;

  return sys_call(&args);
}
/**
 * @brief 写文件
 *
 * @param file
 * @param ptr
 * @param len
 * @return int
 */
int _write(int file, char *ptr, int len) {
  syscall_args_t args;
  args.id = SYS_write;
  args.arg0 = file;
  args.arg1 = (uint32_t)ptr;
  args.arg2 = len;

  return sys_call(&args);
}

/**
 * @brief 关闭文件描述符
 *
 * @param file
 * @return int
 */
int _close(int file) {
  syscall_args_t args;
  args.id = SYS_close;
  args.arg0 = file;

  return sys_call(&args);
}

/**
 * @brief 使文件file位置从dir位置偏移offset
 *
 * @param file
 * @param offset
 * @param dir
 * @return int
 */
int _lseek(int file, int offset, int dir) {
  syscall_args_t args;
  args.id = SYS_lseek;
  args.arg0 = file;
  args.arg1 = offset;
  args.arg2 = dir;

  return sys_call(&args);
}

/**
 * @brief
 *
 * @param file
 * @return int
 */
int _isatty(int file) {
  syscall_args_t args;
  args.id = SYS_isatty;
  args.arg0 = file;

  return sys_call(&args);
}
/**
 * @brief
 *
 * @param file
 * @param st
 * @return int
 */
int _fstat(int file, struct stat *st) {
  syscall_args_t args;
  args.id = SYS_fstat;
  args.arg0 = file;
  args.arg1 = (uint32_t)st;

  return sys_call(&args);
}

/**
 * @brief
 *
 * @param incr
 * @return void*
 */
char *_sbrk(ptrdiff_t incr) {
  syscall_args_t args;
  args.id = SYS_sbrk;
  args.arg0 = (uint32_t)incr;

  return (char *)sys_call(&args);
}

/**
 * @brief 在当前进程的打开文件表中分配新的一项指向该文件描述符对应的文件指针
 *
 * @param file 需要被多次引用的文件指针的文件描述符
 * @return int 新的文件描述符
 */
int dup(int file) {
  syscall_args_t args;
  args.id = SYS_dup;
  args.arg0 = file;

  return sys_call(&args);
}

/**
 * @brief 进程退出的系统调用
 *
 * @param status
 */
void _exit(int status) {
  syscall_args_t args;
  args.id = SYS_exit;
  args.arg0 = status;

  sys_call(&args);
}

/**
 * @brief 回收进程资源
 *
 * @param status
 * @return int
 */
int wait(int *status) {
  syscall_args_t args;
  args.id = SYS_wait;
  args.arg0 = (uint32_t)status;

  return sys_call(&args);
}

// /**
//  * @brief 打开一个目录
//  *
//  * @param path
//  * @return DIR*
//  */
// DIR *opendir(const char *path) {
//   DIR *dir = (DIR *)malloc(sizeof(DIR));
//   if (dir == (DIR *)0) {
//     return (DIR *)0;
//   }

//   syscall_args_t args;
//   args.id = SYS_opendir;
//   args.arg0 = (uint32_t)path;
//   args.arg1 = (uint32_t)dir;

//   int err = sys_call(&args);

//   if (err < 0) {
//     free(dir);
//     return (DIR *)0;
//   }

//   return dir;
// }

/**
 * @brief 读取目录信息得到目录项表
 *
 * @param dir
 * @return struct dirent*
 */
struct dirent *readdir(DIR *dir) {
  syscall_args_t args;
  args.id = SYS_readdir;
  args.arg0 = (uint32_t)dir;
  args.arg1 = (uint32_t) & (dir->dirent);

  int err = sys_call(&args);
  if (err < 0) {
    return (struct dirent *)0;
  }

  return &dir->dirent;
}

// /**
//  * @brief 关闭目录
//  *
//  * @param dir
//  * @return int
//  */
// int closedir(DIR *dir) {
//   syscall_args_t args;
//   args.id = SYS_closedir;
//   args.arg0 = (uint32_t)dir;

//   int err = sys_call(&args);
//   free(dir);

//   return err;
// }

/**
 * @brief 进行io控制
 *
 * @param file
 * @param cmd
 * @param arg0
 * @param arg1
 * @return int
 */
int ioctl(int file, int cmd, int arg0, int arg1) {
  syscall_args_t args;
  args.id = SYS_ioctl;
  args.arg0 = file;
  args.arg1 = cmd;
  args.arg2 = arg0;
  args.arg3 = arg1;

  int err = sys_call(&args);

  return err;
}

/**
 * @brief 删除一个文件
 *
 * @param path
 * @return int
 */
int unlink(const char *path) {
  syscall_args_t args;
  args.id = SYS_unlink;
  args.arg0 = (uint32_t)path;

  int err = sys_call(&args);

  return err;
}