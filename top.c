#include "kernel/types.h"
#include "user/user.h"

#define MAX_PROC 64

struct procinfo {
  int pid;
  char name[16];
  char state[16];
  int ticks;
};

int main() {
  struct procinfo procs[MAX_PROC];
  int i, n;

  // top() syscall fills the array and returns number of processes copied
  n = top((uint64)procs, MAX_PROC);

  printf("Total processes: %d\n", n);
  for (i = 0; i < n; i++) {
    printf("PID: %d\tName: %s\tState: %s\tTicks: %d\n", procs[i].pid, procs[i].name, procs[i].state, procs[i].ticks);

  }

  exit(0);
}
