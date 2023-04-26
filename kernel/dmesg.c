#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "param.h"
#include "stat.h"
#include "proc.h"
#include "strbuf.h"
#include "dmesg.h"

#include <stdarg.h>

#define TICKS_SIZE 20

static struct dmesg_buf {
  struct strbuf msg_buf;
  char msg_buf_mem[MAX_MESSAGE_LEN];

  char user_msg[MAX_MESSAGE_LEN];

  struct strbuf buf;
  char buf_mem[DMESG_SIZE];
  
  struct spinlock lk;
} dmesg_instance;


void
init_dmesg() {
  initlock(&(dmesg_instance.lk), "dmesg");
  buf_init(&(dmesg_instance.msg_buf), dmesg_instance.msg_buf_mem, MAX_MESSAGE_LEN);
  buf_init(&(dmesg_instance.buf), dmesg_instance.buf_mem, DMESG_SIZE);
}

static char digits[] = "0123456789abcdef";

static void
uitoa(uint64 x, char* buf, int base) {
  int dcount = 1;
  for (uint64 x1 = x / base; x1 > 0; x1 /= base, dcount++);

  buf[dcount] = 0;
  for (uint64 x1 = x; dcount > 0; dcount--, x1 /= base)
      buf[dcount - 1] = digits[x1 % base];
}

int
pr_msg(const char* fmt, ...) {
  acquire(&(dmesg_instance.lk));

  char ticks_str[TICKS_SIZE];
  uitoa(uptime(), ticks_str, 10);

  buf_clear(&(dmesg_instance.msg_buf));
  va_list ap;
  va_start(ap, fmt);

  if (buf_vappendf(&(dmesg_instance.msg_buf), 0, fmt, ap) == 0) {
    va_end(ap);
    release(&dmesg_instance.lk);
    return -1;
  }
  va_end(ap);

  if (buf_commit(&dmesg_instance.msg_buf) == 0) {
    release(&dmesg_instance.lk);
    return -1;
  }

  uint64 final_msg_len = 5 + strlen(ticks_str) + strlen(dmesg_instance.msg_buf.buf);
  if (buf_free_space(&dmesg_instance.buf) < final_msg_len) {
    release(&dmesg_instance.lk);
    return -1;
  }

  if (
    buf_appendf(&dmesg_instance.buf, 1, "[%s] %s\n", ticks_str, dmesg_instance.msg_buf.buf) == 0
    && buf_commit(&dmesg_instance.buf) == 0) 
  {
    buf_rollback(&dmesg_instance.buf);
  }

  release(&dmesg_instance.lk);
  return 0;
}

int pr_user_msg(uint64 uptr, int len) {
  if (len >= MAX_MESSAGE_LEN)
    len = MAX_MESSAGE_LEN - 1;
  if (copyin(myproc()->pagetable, dmesg_instance.user_msg, uptr, len) == -1)
    return -1;
  dmesg_instance.user_msg[len] = '\0';
  return pr_msg("%s", dmesg_instance.user_msg);
}

void
print_dmesg() {
  acquire(&dmesg_instance.lk);
  buf_consprint(&dmesg_instance.buf);
  release(&dmesg_instance.lk);
}

int
user_dmesg(uint64 uptr, int len) {
  acquire(&dmesg_instance.lk);
  int i = 0;
  for (int it = dmesg_instance.buf.begin; i < len - 1 && it < dmesg_instance.buf.end; i++, it = (it + 1) % dmesg_instance.buf.capacity) {
    if (copyout(myproc()->pagetable, uptr + i, dmesg_instance.buf.buf + it, 1) == -1)
      return -1;
  }
  if (copyout(myproc()->pagetable, uptr + i, "\0", 1) == -1)
    return -1;
  release(&dmesg_instance.lk);
  return 0;
}
