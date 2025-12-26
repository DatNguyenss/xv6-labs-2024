#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_pgaccess(void)
{
  // lab pgtbl: detect which pages have been accessed
  uint64 va;  // starting virtual address
  int npages; // number of pages to check
  uint64 dstva; // user address to store bitmask
  
  struct proc *p = myproc();
  
  // Parse arguments
  argaddr(0, &va);
  argint(1, &npages);
  argaddr(2, &dstva);
  
  // Limit the number of pages that can be scanned
  // 64 pages = 64 bits = 8 bytes
  if(npages < 0 || npages > 64)
    return -1;
  
  // If no pages to check, just return success with empty bitmask
  if(npages == 0){
    uint64 bitmask = 0;
    if(copyout(p->pagetable, dstva, (char *)&bitmask, sizeof(bitmask)) < 0)
      return -1;
    return 0;
  }
  
  // Check if starting address is valid
  if(va >= p->sz)
    return -1;
  
  // Check for overflow when calculating last page address
  uint64 last_va = va + (npages - 1) * PGSIZE;
  if(last_va < va || last_va >= p->sz)
    return -1;
  
  // Allocate temporary buffer in kernel for bitmask
  uint64 bitmask = 0;
  
  // Check each page
  for(int i = 0; i < npages; i++){
    uint64 page_va = va + i * PGSIZE;
    
    // Find PTE for this virtual address
    pte_t *pte = walk(p->pagetable, page_va, 0);
    if(pte == 0)
      continue; // Page not mapped, skip
    
    // Check if PTE is valid
    if((*pte & PTE_V) == 0)
      continue; // Invalid PTE, skip
    
    // Check if PTE_A (access bit) is set
    if(*pte & PTE_A){
      // Set corresponding bit in bitmask (first page = least significant bit)
      bitmask |= (1UL << i);
      
      // Clear PTE_A after checking
      *pte &= ~PTE_A;
    }
  }
  
  // Copy bitmask to user space
  if(copyout(p->pagetable, dstva, (char *)&bitmask, sizeof(bitmask)) < 0)
    return -1;
  
  return 0;
}
