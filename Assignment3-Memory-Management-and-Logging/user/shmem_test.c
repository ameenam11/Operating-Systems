#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SHM_SIZE 4096

int
main(int argc, char *argv[])
{
  int disable_unmap = (argc > 1 && strcmp(argv[1], "--no-unmap") == 0);
  int parent_pid = getpid();

  char *buf = malloc(SHM_SIZE);
  if(!buf){
    printf("Parent: malloc failed\n");
    exit(1);
  }
  uint64 buf_va = (uint64)buf;
  printf("Parent: malloced buffer at %p\n", buf);

  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    printf("Child: heap before mapping: %p\n", sbrk(0));

    uint64 child_va = map_shared_pages(parent_pid, getpid(), buf_va, SHM_SIZE);
    if(child_va < 0){
      printf("Child: map_shared failed\n");
      exit(1);
    }
    printf("Child: mapped at %p, heap now: %p\n", (void*)child_va, sbrk(0));

    strcpy((char*)child_va, "hello owner");

    if(!disable_unmap){
      printf("Child: unmapping shared page at %p\n", (void*)child_va);
      if(unmap_shared_pages(getpid(), child_va, SHM_SIZE) < 0)
        printf("Child: unmap_shared failed\n");
      printf("Child: heap after unmapping: %p\n", sbrk(0));
    } else {
      printf("Child: --no-unmap, exiting without unmap\n");
    }

    exit(0);
  }

  wait(0);

  printf("Parent: after child exit, content = \"%s\"\n", buf);
  printf("Parent: heap before unmapping: %p\n", sbrk(0));

  printf("Parent: unmapping own shared page at %p\n", buf);
  if(unmap_shared_pages(getpid(), buf_va, SHM_SIZE) < 0)
    printf("Parent: unmap_shared failed\n");
  printf("Parent: heap after unmapping: %p\n", sbrk(0));

  free(buf);
  exit(0);
}