#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    printf("usage: trace mask command [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  if (trace(mask) < 0) {
    printf("trace: syscall failed\n");
    exit(1);
  }

  // Chạy chương trình mục tiêu, argv[2] là tên, argv[2...] là args
  exec(argv[2], &argv[2]);

  // Nếu exec thất bại
  printf("trace: exec %s failed\n", argv[2]);
  exit(1);
}
