#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define NPAGES 4

int
main(void)
{
  char *p;
  uint64 mask;

  printf("pgaccess_test starting\n");

  // Cấp phát NPAGES trang
  p = sbrk(NPAGES * PGSIZE);
  if (p == (char *)-1) {
    printf("pgaccess_test: sbrk failed\n");
    exit(1);
  }

  //
  // Test 1: truy cập page 0 và page 2
  //
  p[0] = 1;
  p[2 * PGSIZE] = 1;

  mask = 0;
  if (pgaccess(p, NPAGES, &mask) < 0) {
    printf("pgaccess_test: pgaccess failed\n");
    exit(1);
  }

  printf("mask after first access = 0x%lx\n", mask);

  // bit 0 và bit 2 phải được set: 0b0101 = 0x5
  if (mask != 0x5) {
    printf("pgaccess_test: wrong mask (expected 0x5)\n");
    exit(1);
  }

  //
  // Test 2: kiểm tra clear bit PTE_A
  // Không truy cập thêm trang nào
  //
  mask = 0;
  if (pgaccess(p, NPAGES, &mask) < 0) {
    printf("pgaccess_test: pgaccess failed\n");
    exit(1);
  }

  printf("mask after second access = 0x%lx\n", mask);

  // Sau khi clear PTE_A, mask phải bằng 0
  if (mask != 0) {
    printf("pgaccess_test: access bit not cleared\n");
    exit(1);
  }

  //
  // Test 3: truy cập tất cả các trang
  //
  for (int i = 0; i < NPAGES; i++) {
    p[i * PGSIZE] = i;
  }

  mask = 0;
  if (pgaccess(p, NPAGES, &mask) < 0) {
    printf("pgaccess_test: pgaccess failed\n");
    exit(1);
  }

  printf("mask after accessing all pages = 0x%lx\n", mask);

  // Tất cả bit phải được set: 0b1111 = 0xf
  if (mask != 0xf) {
    printf("pgaccess_test: wrong mask (expected 0xf)\n");
    exit(1);
  }

  //
  // Test 4: fork - mỗi process có page table riêng
  //
  int pid = fork();
  if (pid < 0) {
    printf("pgaccess_test: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // Child chỉ truy cập page 1
    p[1 * PGSIZE] = 1;

    mask = 0;
    pgaccess(p, NPAGES, &mask);

    if (mask != 0x2) {
      printf("pgaccess_test: child wrong mask\n");
      exit(1);
    }
    exit(0);
  } else {
    wait(0);

    mask = 0;
    pgaccess(p, NPAGES, &mask);

    // Parent không truy cập thêm → mask phải = 0
    if (mask != 0) {
      printf("pgaccess_test: parent isolation failed\n");
      exit(1);
    }
  }

  printf("pgaccess_test: OK\n");
  exit(0);
}
