/**
 * @file first_task.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 第一个任务进程，实现与操作系统的代码隔离
 * @version 0.1
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "applib/lib_syscall.h"
#include "core/task.h"
#include "tools/log.h"

int first_main(void) {
  // 为每个tty设备创建一个进程
  for (int i = 0; i < 1; ++i) {
    int pid = fork();
    if (pid < 0) {
      print_msg("create shell failed.", 0);
      break;
    } else if (pid == 0) {
      char tty_num[] = "/dev/tty?";
      tty_num[sizeof(tty_num) - 2] = i + '0';
      char* const argv[] = {tty_num, 0};
      execve("shell.elf", argv, 0);
      while (1) {
        msleep(1000);
      }
    }
  }

  for (;;) {
    // 回收所有孤儿进程
    int status = 0;
    wait(&status);
  }

  // while (1) {
  //   print_msg("hello world %d\n", 10);
  //   msleep(1000);
  // }

  return 0;
}