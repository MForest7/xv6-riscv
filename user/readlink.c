#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[MAXPATH];

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("error: must be exactly two arguments: $ readlink <linkpath>\n");
        exit(1);
    }

    int error_code = readlink(argv[1], buf);
    if (error_code) {
        printf("error: code is %d\n", error_code);
        exit(1);
    }

    printf("%s\n", buf);
    exit(0);
}
