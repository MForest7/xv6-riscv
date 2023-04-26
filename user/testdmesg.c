#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SIZE 8192

char buf[BUF_SIZE];

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
    while (c != '\n') {
        read(id, &c, 1);
    }
    buf[length] = 0;
    return length;
}

int
main(int argc, char *argv[])
{
    while (1) {
        char cmd = 0;
        if (read(0, &cmd, 1) <= 0) {
            exit(0);
        }

        char delimiter;
        int len;
        read(0, &delimiter, 1);
        if (delimiter != '\n') {
            len = receive(0, buf, BUF_SIZE);

            if (delimiter != ' ') {
                printf("unknown command\n");
                continue;
            }
        }

        if (cmd == 'm') {
            if (delimiter != ' ') {
                printf("message expected\n");
                continue;
            }
            prmsg(buf, len);
        } else if (cmd == 'p') {
            dmesg(buf, BUF_SIZE);
            printf("%s", buf);
        } else if (cmd == 'q') {
            exit(0);
        } else if (cmd == 'h') {
            printf(" m <message> -- add message to dmesg\n");
            printf(" p           -- print dmesg\n");
            printf(" q           -- quit\n");
            printf(" h           -- show help\n");
        } else if (cmd != ' ' && cmd != '\n') {
            printf("unknown command\n");
        }
    }
}
