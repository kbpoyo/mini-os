
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // optind是下一个要处理的元素在argv中的索引
  // 当没有选项时，变为argv第一个不是选项元素的索引。
  int mem_number = 0;
  int task_number = 0;
  int ch;
  while ((ch = getopt(argc, argv, "m:t:h")) != -1) {
    switch (ch) {
      case 'h':
        puts("Test os memory or task.");
        puts("Usage: os_test [-m mem_number(M)] [-t task_number]");
        optind = 1;  // getopt需要多次调用，需要重置
        return 0;
      case 'm':
        mem_number = atoi(optarg);
        break;
      case 't':
        task_number = atoi(optarg);
        break;
      case '?':
        if (optarg) {
          fprintf(stderr, "Unknown option: -%s\n", optarg);
        }
        optind = 1;  // getopt需要多次调用，需要重置
        return -1;
    }
  }

  // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
  if (optind > argc) {
    fprintf(stderr, "parameter is empty.\n");
    optind = 1;  // getopt需要多次调用，需要重置
    return -1;
  }

  optind = 1;  // getopt需要多次调用，需要重置

  if (mem_number > 0) {
    printf("Test os memory, mem_number: %d\n", mem_number);

    int **ptr_array = (int **)malloc(mem_number * sizeof(int *));
    if (ptr_array == NULL) {
      printf("malloc failed.\n");
      return -1;
    }
    memset(ptr_array, 0, mem_number * sizeof(int *));

    if (mem_number > 64) {
      printf("mem_number is too large, max is 64.\n");
      return -1;
    }
    for (int i = 0; i < mem_number; i++) {
      ptr_array[i] = malloc(1024 * 1024);
      if (ptr_array[i] == NULL) {
        printf("malloc failed, i: %d\n", i);
        break;
      }

      printf("malloc success, %dM.\n", i);
    }

    for (int i = 0; i < mem_number; i++) {
      if (ptr_array[i] != NULL) {
        free(ptr_array[i]);
        printf("free success, iM: %d\n", i);
      }
    }

    printf("Test os memory success.\n");
  } else if (task_number > 0) {
    printf("Test os task, task_number: %d\n", task_number);

    int pid = -1;
    for (int i = 0; i < task_number; i++) {
      pid = fork();
      if (pid == 0) {
        break;
      } else if (pid < 0) {
        printf("fork failed.\n");
        return -1;
      }
    }

    if (pid == 0) {
      for (int i = 0; i < 10; ++i) {
        printf("child process, pid: %d\n", getpid());
        msleep(1000);
      }

      printf("child process, pid: %d, died.\n", getpid());
      return 0;
    } else if (pid > 0) {
      for (int i = 0; i < task_number; ++i) {
        int status = 0;
        pid_t child_pid = wait(&status);
        if (child_pid > 0) {
          printf("recovery child success chpid = %d, status = %d\n", child_pid,
                 status);
        }
      }

      printf("parent process, pid: %d, died.\n", getpid());
      printf("Test os task success.\n");
    }
  }
}
