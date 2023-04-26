#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/dmesg.h"
#include "user/user.h"

static char buf[DMESG_P * PGSIZE];

int
main(int argc, char *argv[])
{
    dmesg(buf, DMESG_P * PGSIZE);
    printf("%s", buf);

    return 0;
}
