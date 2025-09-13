#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N 4096
#define SHM_SIZE 4096
#define NCHILDREN 4

int
main(int argc, char *argv[])
{

  char *buf = malloc(SHM_SIZE);
  if (!buf) {
    printf("Parent: malloc failed\n");
    exit(1);
  }
  memset(buf, 0, SHM_SIZE);

  int *pids = malloc(N * sizeof(int));
  if (!pids) {
    printf("Parent: Failed to allocate pids array\n");
    exit(1);
  }

  uint64 buf_va = (uint64)buf;
  printf("Parent: malloced buffer at %p\n", buf);

  int parent_pid = getpid();
  printf("Parent: PID is %d\n", parent_pid);
  int fork_ret = forkn(NCHILDREN, (uint64)pids);
  
  if(fork_ret == 0){
    // parent process
    for (int j = 0; j < NCHILDREN; j++) {
    int child_pid = pids[j];
    printf("Parent: Child %d has PID %d\n", j, child_pid);
    }

    for (int offset = 0; offset < SHM_SIZE;) {
    
    uint32 hdr = *(uint32*)(buf_va + offset);
    if (hdr == 0) {
      offset += 4;
      printf("offset %d: empty header\n", offset);
      continue;
    }
    int pid = hdr >> 16;
    int len = hdr & 0xFFFF;
    char* msg = (char*)(buf_va + offset + sizeof(uint32));
    printf("Parent: Message from child with PID %d: %s, with offset %d\n", pid, msg, offset);
    offset += 4 + len;
    offset = (offset + 3) & ~3; 
    }

    exit(0);
  }

  // child processes
  uint64 va = map_shared_pages(parent_pid, getpid(), buf_va, SHM_SIZE);
  
  char *msg;
  int   len;
  if (getpid() % 2 == 0) {
    msg = "hello world";
    len = strlen(msg);         
  } else {
    len = SHM_SIZE/4; 
    msg = malloc(len);
    if(!msg){ printf("malloc failed\n"); exit(1); }
    memset(msg, 'X', len);
  }

  for (int offset = 0; offset < SHM_SIZE;) {
    uint32 newhdr = (getpid() << 16) | len;
    
    if(offset + len + 4 < SHM_SIZE) {
    if(__sync_val_compare_and_swap((int*)(va + offset), 0, newhdr) == 0){
      void *datap = (void*)(va + offset + sizeof(newhdr));
      memcpy(datap, msg, len);
      offset += len + 4;
      offset = (offset + 3) & ~3;
    }else{
      uint32 hdr = *(uint32*)(va + offset);
      int temp_len = hdr & 0xFFFF;
      offset += temp_len + 4; 
      offset = (offset + 3) & ~3;
    }
    }else{
    break;
    }
  }

  exit(0);
}