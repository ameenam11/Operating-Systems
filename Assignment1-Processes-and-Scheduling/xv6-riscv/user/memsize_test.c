#include "kernel/types.h"
#include "user/user.h"

int main() {
    printf("Memory usage before allocation: %d bytes\n", memsize());

    char *buffer = malloc(20 * 1024); // 20k bytes

    printf("Memory usage after allocation: %d bytes\n", memsize());

    free(buffer);

    printf("Memory usage after release: %d bytes\n", memsize());

    exit(0, "");
}