#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("you have to assign the number of initial processes\n");
    exit(1);
  }

  int n = atoi(argv[1]);

  int tid = tournament_create(n);
  if (tid < 0) {
    printf("tournament_create failed\n");
    exit(1);
  }

  // Enter critical section
  if (tournament_acquire() < 0) {
    printf("tournament_acquire failed\n");
    exit(1);
  }

  printf("Process %d (PID %d) in critical section\n", tid, getpid());

  if (tournament_release() < 0) {
    printf("tournament_release failed\n");
    exit(1);
  }

  if (tid == 0) {// Parent waits for children
  for (int i = 1; i < n; i++)
    wait(0);

  printf("Destroying tournament tree...\n");
  if (tournament_destroy() < 0) {
    printf("Failed to destroy tournament\n");
    exit(1);
  }
}
  exit(0);
}
