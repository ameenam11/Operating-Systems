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

uint64 sys_map_shared_pages(void)
{
  uint64 size;
  int src_pid, dst_pid, src_va;
  struct proc *src_proc, *dst_proc;
  argint(0, &src_pid);
  argint(1, &dst_pid);
  if (src_pid < 0 || dst_pid < 0 || src_pid == dst_pid) {
    return -1; // Invalid process IDs
  }
  argint(2, &src_va);
  argint(3, (int *)&size);

  if (size <= 0 || src_va % PGSIZE != 0) {
    return -1; // Invalid arguments
  }

  src_proc = find_proc_by_pid(src_pid);
  dst_proc = find_proc_by_pid(dst_pid);
  if (src_proc == 0 || dst_proc == 0) {
    return -1; // Source or destination process not found
  }

  return map_shared_pages(src_proc, dst_proc, src_va, size);
}

uint64 sys_unmap_shared_pages(void){
  uint64 size;
  int pid, addr;
  struct proc *p;
  argint(0, &pid);
  argint(1, &addr);
  argint(2, (int *)&size);

  p = find_proc_by_pid(pid);
  if (p == 0) {
    return -1; // Process not found
  }

  return unmap_shared_pages(p, addr, size);
}

uint64
sys_getpagesize(void)
{
  return myproc()->sz;
}


uint64
sys_forkn(void)
{
  uint64 n;
  uint64 pids;
  argaddr(0, &n);
  argaddr(1, &pids);
  return forkn(n, pids);
}

