
#include "main.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#include "applib/lib_syscall.h"

int main(int argc, char** argv) {
  if (argc <= 0) {
    puts("Usage: multi -n count");
    return 0;
  }

  // optind是下一个要处理的元素在argv中的索引
  // 当没有选项时，变为argv第一个不是选项元素的索引。
  int count = 0;  // 缺省只打印一次
  int ch;
  while ((ch = getopt(argc, argv, "n:h")) != -1) {
    switch (ch) {
      case 'h':
        puts("run mutli tasks.");
        puts("Usage: multi -n count");
        optind = 1;  // getopt需要多次调用，需要重置
        return 0;
      case 'n':
        count = atoi(optarg);
        break;
      case '?':
        if (optarg) {
          fprintf(stderr, "Unknown option: -%s\n", optarg);
        }
        optind = 1;  // getopt需要多次调用，需要重置
        return -1;
    }
  }

  if (count <= 0 || count >= 126) {
    fprintf(stderr, "can't run %d tasks\n", count);
    optind = 1;
    return -1;
  }

  int pid = -1;
  int i;
  for (i = 0; i < count; ++i) {
    pid = fork();
    if (pid == 0) {
      break;
    }
  }
  int time = 0;
  while (1) {
    if (pid > 0) {
      printf(
          "==================== this is parent pid = %d "
          "\t====================\n",
          getpid());
      msleep(1000);
      if (time++ >= 10) {
        printf("==================== parent died.\t====================\n");
        return 0;
      }
    } else {
      printf(
          "-------------------- this is child %d, pid = %d "
          "\t--------------------\n",
          i, getpid());
      msleep(500);
      if (time++ >= 10) {
        printf("==================== child %d died.\t====================\n",
               i);

        return 0;
      }
    }
  }

  optind = 1;  // getopt需要多次调用，需要重置

  return 0;
}
