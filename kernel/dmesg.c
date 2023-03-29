#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "param.h"
#include "stat.h"
#include "proc.h"

#define TICKS_SIZE 20

static char buf[DMESG_P * PGSIZE];
static int end;
static struct sleeplock lk;

void
init_dmesg() {
    initsleeplock(&lk, "dmesg");
}

void 
append(const char* str) {
    strncpy(buf + end, str, strlen(str)); 
    end += strlen(str);
}

void
uitoa(uint64 x, char* buf, int base) {
    int digits = 1;
    for (uint64 x1 = x / base; x1 > 0; x1 /= base, digits++);

    buf[digits] = 0;
    for (uint64 x1 = x; digits > 0; digits--, x1 /= base)
        buf[digits - 1] = (x1 % base) + '0';
}

void
pr_msg(uint64 addr, int len) {
    acquiresleep(&lk);

    char ticks_str[TICKS_SIZE];
    uitoa(uptime(), ticks_str, 10);

    if (end + len + strlen(ticks_str) + 10 < DMESG_P * PGSIZE) {
        append("[time: ");
        append(ticks_str);
        append("] ");
        copyin(myproc()->pagetable, buf + end, addr, len);
        end += len;
        append("\n");
    }

    releasesleep(&lk);
}

void
dmesg() {
    acquiresleep(&lk);
    printf("%s", buf);
    releasesleep(&lk);
}
