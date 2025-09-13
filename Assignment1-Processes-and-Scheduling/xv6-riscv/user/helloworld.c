#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    char *p = "Hello World xv6!\n";
    printf("%s", p);
    exit(0, "");
}
