#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define N 65536
#define NCHILDREN 4
#define CHUNK_SIZE (N / NCHILDREN)

int
main(int argc, char *argv[])
{
    int *pids = malloc(N * sizeof(int));
    if (!pids) {
        printf("Failed to allocate pids array\n");
        exit(1, "");
    }

    int fork_ret = forkn(NCHILDREN, (uint64)pids);

    if (fork_ret == 0) {
        // Parent
        int *n = malloc(sizeof(int));
        int *statuses = malloc(sizeof(int) * NCHILDREN);

        if (!n || !statuses) {
            printf("Parent: malloc failed\n");
            exit(1, "");
        }

        waitall((uint64)n, (uint64)statuses);

        for (int i = 0; i < *n; i++) {
            printf("Parent: Child pid is %d\n", pids[i]);
        }

        int sum = 0;
        for (int i = 0; i < *n; i++) {
            sum += statuses[i];
        }

        printf("Parent: Sum of all children is %d\n", sum);

        free(n);
        free(statuses);
        free(pids);
        exit(0, "");

    } else if (fork_ret > 0) {
        // Child
        printf("Child: fork returned the number %d\n", fork_ret);

        int start = (fork_ret - 1) * CHUNK_SIZE;
        int end = fork_ret * CHUNK_SIZE;
        int sum = 0;

        for (int i = start; i < end; i++) {
            sum += i;
        }

        exit(sum, "");

    } else {
        // Error
        printf("Error: forkn failed\n");
        free(pids);
        exit(1, "");
    }

    // Should never reach here
    free(pids);
    exit(0, "");
}
