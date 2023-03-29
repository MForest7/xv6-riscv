#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
read_int(uint64 fd) {
    const int BUF_SIZE = 16;
    char buf[BUF_SIZE];
    char next = 0;

    for (int i = 0; 1; i++) {
        if (read(fd, &next, 1) > 0 && '0' <= next && next <= '9') {
            buf[i] = next;
        } else {
            buf[i] = 0; break;
        }
    }
    return atoi(buf);
}

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("must be exactly one argument: size of dynamically allocated memory");
    }

    int buf_size = atoi(argv[1]);
    char* a = (char*)malloc(buf_size);
    if (a == 0) {
        exit(1);
    }

    while (1) {
        char cmd = 0;
        if (read(0, &cmd, 1) <= 0) {
            free(a);
            exit(0);
        }
        if (cmd == 'b') {
            char space;
            read(0, &space, 1);
            if (space != ' ') {
                printf("unknown command\n"); 
                continue;
            }

            int x = read_int(0);
            if (x >= buf_size) {
                printf("invalid index");
            } else {
                printf("%d-th byte is %d\n", x, a[x]);
            }
        } else if (cmd == 'p') {
            vmprint();
        } else if (cmd == 'a') {
            pgaccess();
        } else if (cmd == 'q') {
            free(a);
            exit(0);
        } else if (cmd != ' ' && cmd != '\n') {
            printf("unknown command\n");
        }
    }
}
