#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SIZE 512

int receive(int id, char* buf, int max_len) {
    char c;
    int length = 0;
    while (1) {
        read(id, &c, 1);
        if (c == '\n')
            break;
        if (length + 1 < max_len)
            buf[length++] = c;
    }
    buf[length] = 0;
    return length;
}

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];

    while (1) {
        char cmd = 0;
        if (read(0, &cmd, 1) <= 0) {
            exit(0);
        }
        if (cmd == 'm') {
            char space;
            read(0, &space, 1);
            if (space != ' ') {
                printf("unknown command\n"); 
                continue;
            }

            int len = receive(0, buf, BUF_SIZE);
            prmsg(buf, len);
        } else if (cmd == 'p') {
            dmesg(buf, BUF_SIZE);
            printf("%s", buf);
        } else if (cmd == 'q') {
            exit(0);
        } else if (cmd != ' ' && cmd != '\n') {
            printf("unknown command\n");
        }
    }
}
