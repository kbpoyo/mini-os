#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#include "applib/lib_syscall.h"
void do_undef(void) {
  __asm__ __volatile__(".word 0xeeadc0de\n");  // 异常指令
}

int main(int argc, char **argv) {
  do_undef();

  return 0;
}
