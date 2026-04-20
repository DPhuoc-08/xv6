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
  uint64 va;      // starting virtual address
  int npages;     // number of pages to check
  uint64 abitsaddr; // user address for bitmask result

  argaddr(0, &va);
  argint(1, &npages);
  argaddr(2, &abitsaddr);

  // limit the number of pages that can be scanned
  if(npages > 32)
    npages = 32;
  if(npages < 0)
    return -1;

  struct proc *p = myproc();
  unsigned int mask = 0;

  for(int i = 0; i < npages; i++){
    uint64 addr = va + i * PGSIZE;
    pte_t *pte = walk(p->pagetable, addr, 0);
    if(pte == 0)
      continue;
    if((*pte & PTE_V) == 0)
      continue;
    if(*pte & PTE_A){
      mask |= (1 << i);
      // clear the accessed bit so next call can detect new accesses
      *pte &= ~PTE_A;
    }
  }

  // copy the bitmask to user space
  if(copyout(p->pagetable, abitsaddr, (char *)&mask, sizeof(mask)) < 0)
    return -1;

  return 0;
}
