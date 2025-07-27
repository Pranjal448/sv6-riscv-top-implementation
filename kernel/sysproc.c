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


extern struct proc proc[NPROC];
// defined in proc.c, proc[NPROC] is an array containing all processes

uint64
sys_top(void)
{
  uint64 uaddr;
  int max;

  argaddr(0, &uaddr);   // (uint64)procs
  argint(1, &max); // MAXPROCS (which is 64)

  if (max > NPROC)
    max = NPROC;

  struct procinfo {
    int pid;
    char name[16];
    char state[16];
    int ticks;
  } kprocs[NPROC];
  // this is a buffer   
  // you can't copy directly to user, you must first create information here and then copy to user

  int i = 0; // how many process we have added to output array
  for (int j = 0; j < NPROC && i < max; j++) { 
    // goes over all kernel process

    struct proc *p = &proc[j];
    // for jth process

    if (p->state != UNUSED) {
      // skip if process is unused

      kprocs[i].pid = p->pid;
      // copy process id

      safestrcpy(kprocs[i].name, p->name, sizeof(kprocs[i].name));
      // copy process name

      kprocs[i].ticks = p->ticks;
      // copy no. of ticks

      switch (p->state) {
        case SLEEPING:
          safestrcpy(kprocs[i].state, "SLEEPING", sizeof(kprocs[i].state));
          break;
        case RUNNING:
          safestrcpy(kprocs[i].state, "RUNNING", sizeof(kprocs[i].state));
          break;
        case ZOMBIE:
          safestrcpy(kprocs[i].state, "ZOMBIE", sizeof(kprocs[i].state));
          break;
        case RUNNABLE:
          safestrcpy(kprocs[i].state, "RUNNABLE", sizeof(kprocs[i].state));
          break;
        default:
          safestrcpy(kprocs[i].state, "OTHER", sizeof(kprocs[i].state));
      }
      // copy process state
      
      i++;
      
    }
  }

  if (copyout(myproc()->pagetable, uaddr, (char *)kprocs, i * sizeof(kprocs[0])) < 0) 
    return -1;
  // copyout function transfers data from kernel memory to user memory
  // myproc()->pagetable : the current process's page table, used to access user memory 
  // uaddr : desination virtual address (in user space) to write to ,,,,,, points to procs[] ,,,,, top(--->(uint64)procs<---, MAX_PROC);
  // kprocs : souce buffer (what to copy)
  // i * sizeof(kprocs[0]) : size (in bytes) you want to copy

  return i;
}
