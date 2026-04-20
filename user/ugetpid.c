#include "kernel/types.h" 
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/usyscall.h"

int
ugetpid(void)
{
  struct usyscall *u = (struct usyscall *)USYSCALL;
  return u->pid;
}