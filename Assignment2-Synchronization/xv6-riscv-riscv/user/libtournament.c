#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int *locks = 0;
static int tournamentID = 0;
static int height = 0;
static int totalProcesses = 0;

int power_of_two(int k) {
  int result = 1;
  for (int i = 0; i < k; i++) {
    result *= 2;
  }
  return result;
}

int tournament_create(int processes){
    if(processes != 2 && processes != 4 && processes != 8 && processes != 16){
        fprintf(2, "tournament_create: invalid number of processes\n");
        return -1;
    }
    else{
          totalProcesses = processes;
  height = 0;

  int temp = processes;
  while (temp > 1) {
    temp /= 2;
    height++;
  }

  int num_locks = processes - 1; // number of locks is represented by the number of internal nodes in the tree
  locks = malloc(sizeof(int) * num_locks);
  if (!locks)
    return -1;

  // Create locks 
  for (int i = 0; i < num_locks; i++) {
    int lock_id = peterson_create();
    if (lock_id < 0)
      return -1;
    locks[i] = lock_id;
  }

  // Fork processes and assign tournament ID
  for (int i = 1; i < processes; i++) {
    int pid = fork();
    if (pid < 0)
      return -1;
    if (pid == 0) {
      tournamentID = i;
      break;
    }
  }
  return tournamentID;
    }
}

int tournament_acquire(void)
{
  if (tournamentID < 0 || locks == 0)
    return -1;

  for (int j = height - 1; j >= 0; j--) {
    int k = height - j - 1;

    // Compute role
    int div_k = power_of_two(k);
    int role = (tournamentID / div_k) % 2;

    // Compute lock index
    int div_lvl = power_of_two(height - j);
    int subtree_index = tournamentID / div_lvl;
    int offset = power_of_two(j) - 1;
    int lock_at_level_j = subtree_index + offset;

    if (peterson_acquire(locks[lock_at_level_j], role) < 0)
      return -1;
  }

  return 0;
}



int tournament_release(void)
{
  if (tournamentID < 0 || locks == 0)
    return -1;

  for (int j = 0; j < height; j++) {
    int k = height - j - 1;

    int div_k = power_of_two(k);
    int role = (tournamentID / div_k) % 2;

    int div_lvl = power_of_two(height - j);
    int subtree_index = tournamentID / div_lvl;
    int offset = power_of_two(j) - 1;
    int lock_at_level_j = subtree_index + offset;

    if (peterson_release(locks[lock_at_level_j], role) < 0)
      return -1;
  }

  return 0;
}


int tournament_destroy(void)
{
  if (locks == 0 || totalProcesses <= 1)
    return -1;

  for (int i = 0; i < totalProcesses - 1; i++) {
    if (peterson_destroy(locks[i]) < 0)
      return -1;
  }

  free(locks);
  locks = 0;
  tournamentID = 0;
  height = 0;
  totalProcesses = 0;

  return 0;
}

