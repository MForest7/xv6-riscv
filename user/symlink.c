#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("error: must be exactly two arguments: $ testsymlink <target> <linkpath>\n");
        printf("  where <target> and <linkpath> are system call arguments\n");
        exit(1);
    }
    
    int error_code = symlink(argv[1], argv[2]);
    if (error_code) {
        printf("error: code is %d\n", error_code);
        exit(1);
    }

    printf("symlink created\n");
    exit(0);
}
