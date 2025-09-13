#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "petersonlock.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

struct petersonlock peterson_locks[15];

int 
peterson_create(void){
    for (int i = 0; i < 15; i++) {
        if (__sync_lock_test_and_set(&peterson_locks[i].locked, 1) == 0) {
          __sync_synchronize();
          peterson_locks[i].flags[0] = 0;
          peterson_locks[i].flags[1] = 0;
          peterson_locks[i].turn = 0;
          return i;
        }
      }
      return -1;
}

int
peterson_acquire(int lock_id, int role){
    struct petersonlock *lock = &peterson_locks[lock_id];
    int other = 1 - role;

  lock->flags[role] = 1;
  __sync_synchronize();
  lock->turn = other;
  __sync_synchronize();

  while (lock->flags[other] && lock->turn == other) {
    yield(); 
  }
  return 0;
}

int peterson_release(int lock_id, int role){
    __sync_synchronize();
    peterson_locks[lock_id].flags[role] = 0;
    __sync_synchronize();
    return 0;
}

int peterson_destroy(int lock_id){
    __sync_synchronize();
    peterson_locks[lock_id].locked = 0;
    return 0;
}