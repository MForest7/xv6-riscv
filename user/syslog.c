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
    if (argc != 3) {
        printf("error: must be exactly two arguments: $ syslog <event_type_id> <ticks>\n");
        printf("  where:\n");
        printf("    <event_type_id>:\n");
        printf("      1 - interrupts\n");
        printf("      2 - swithes\n");
        printf("      3 - system calls\n");
        printf("    <ticks> - number of ticks when logging must stop\n");
        printf("      (0 to stop immediately, 1 to log forever)");
        exit(1);
    }

    int event_type_id = atoi(argv[1]);
    int ticks = atoi(argv[2]);

    if (event_type_id < 1 || 3 < event_type_id) {
        printf("expected <event_type_id> from 1 to 3\n");
        exit(1);
    }

    setlogging(event_type_id, ticks);
    exit(0);
}
