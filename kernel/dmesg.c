#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "param.h"
#include "stat.h"
#include "proc.h"

#include <stdarg.h>

#define TICKS_SIZE 20
#define MAX_MESSAGE_LEN 512

static char msg_buf[MAX_MESSAGE_LEN];
static int msg_end;
static char user_buf[MAX_MESSAGE_LEN];

static char buf[DMESG_P * PGSIZE];
static int begin, end;
static struct spinlock lk;

void
init_dmesg() {
    initlock(&lk, "dmesg");
}

static char digits[] = "0123456789abcdef";

static void 
putchar(char c) {
    if (MAX_MESSAGE_LEN <= msg_end + 1)
        return;
    msg_buf[msg_end++] = c;
}

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    putchar(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  putchar('0');
  putchar('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putchar(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

void 
append(const char* s) {
  do {
    buf[end % (DMESG_P * PGSIZE)] = *s++;
    end++;
  } while (*s);
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
pr_msg(const char* fmt, ...) {
  acquire(&lk);

  char ticks_str[TICKS_SIZE];
  uitoa(uptime(), ticks_str, 10);

  va_list ap;
  int i, c;
  char *s;
  msg_end = 0;

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      putchar(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        putchar(*s);
      break;
    case '%':
      putchar('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      putchar('%');
      putchar(c);
      break;
    }
  }
  va_end(ap);
  msg_buf[msg_end] = '\0';

  int final_len = msg_end + strlen(ticks_str) + 5;
  if (final_len < DMESG_P * PGSIZE) {
    while (final_len + end > begin + DMESG_P * PGSIZE) {
      while (buf[begin % (DMESG_P * PGSIZE)] != '\0')
        begin++;
      begin++;
    }

    append("[");
    append(ticks_str);
    append("] ");
    
    append(msg_buf);
    append("\n\0");
  }

  release(&lk);
}

void pr_user_msg(uint64 uptr, int len) {
  if (len >= MAX_MESSAGE_LEN)
    len = MAX_MESSAGE_LEN - 1;
  copyin(myproc()->pagetable, user_buf, uptr, len);
  user_buf[len] = '\0';
  pr_msg("%s", user_buf);
}

void
dmesg() {
    acquire(&lk);
    printf("%s", buf);
    release(&lk);
}

void 
user_dmesg(uint64 uptr, int len) {
  acquire(&lk);
  int i = 0;
  for (int it = begin; i < len && it < end; i++, it++)
    user_buf[i] = buf[it % (DMESG_P * PGSIZE)];
  user_buf[i] = '\0';
  copyout(myproc()->pagetable, uptr, user_buf, i);
  release(&lk);
}
