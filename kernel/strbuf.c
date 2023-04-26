#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "strbuf.h"

#include <stdarg.h>

static char digits[] = "0123456789abcdef";

void 
buf_init(struct strbuf* buf, char* mem, uint64 capacity) {
    if (capacity < 2)
        panic("invalid strbuf");
    
    buf->begin = 0;
    buf->end = 0;
    buf->savepoint = 0;
    buf->buf = mem;
    buf->capacity = capacity;
}

void
buf_clear(struct strbuf* buf) {
    buf->begin = 0;
    buf->end = 0;
    buf->savepoint = 0;
}

static uint64
buf_popstr(struct strbuf* buf) {
    uint64 freed = 0;
    while (buf->buf[buf->begin]) {
        if (buf->begin == buf->savepoint)
            return freed;
        uint64 next = buf->begin = (buf->begin + 1) % buf->capacity;
        buf->begin = next;
        freed++;
    }
    return freed;
}

static int 
putchar(struct strbuf* buf, char c, int forced) {
    uint64 next = (buf->end + 1) % buf->capacity;
    if (next == buf->begin) {
        if (forced) {
            buf_popstr(buf);
            if (next == buf->begin)
                return 0;
        } else {
            return 0;
        }
    }
    buf->buf[buf->end++] = c;
    return 1;
}

uint64
buf_free_space(struct strbuf* buf) {
    if (buf->begin <= buf->end)
        return buf->capacity - 1 - (buf->end - buf->begin);
    else
        return buf->begin - 1 - buf->end;
}

int
buf_commit(struct strbuf* buf) {
    int success = putchar(buf, 0, 1);
    buf->savepoint = buf->end;
    return success;
}

void
buf_rollback(struct strbuf* buf) {
    buf->end = buf->savepoint;
}

static int
printint(struct strbuf* buf, int xx, int base, int sign, int forced)
{
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  do {
    if (putchar(buf, digits[x % base], forced) == 0) return 0;
  } while((x /= base) != 0);

  if(sign) {
    if (putchar(buf, '-', forced) == 0) return 0;
  }

  return 1;
}

static int
printptr(struct strbuf* buf, uint64 x, int forced)
{
  int i;
  if (putchar(buf, '0', forced) == 0) return 0;
  if (putchar(buf, 'x', forced) == 0) return 0;
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    if (putchar(buf, digits[x >> (sizeof(uint64) * 8 - 4)], forced) == 0) return 0;
  return 1;
}

static int 
printstr(struct strbuf* buf, const char* s, int forced) {
  for (; *s; s++) {
    if (putchar(buf, *s, forced) == 0) return 0;
  }
  return 1;
}

int
buf_vappendf(struct strbuf* buf, int forced, const char* fmt, va_list ap) {
  int i, c;
  char *s;

  if (fmt == 0)
    panic("null fmt");

  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      if (putchar(buf, c, forced) == 0) {
        buf_rollback(buf);
        return 0;
      }
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    
    int success = 1;
    switch(c){
    case 'd':
      success &= printint(buf, va_arg(ap, int), 10, 1, forced);
      break;
    case 'x':
      success &= printint(buf, va_arg(ap, int), 16, 1, forced);
      break;
    case 'p':
      success &= printptr(buf, va_arg(ap, uint64), forced);
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      success &= printstr(buf, s, forced);
      break;
    case '%':
      success &= putchar(buf, '%', forced);
      break;
    default:
      // Print unknown % sequence to draw attention.
      success &= putchar(buf, '%', forced);
      success &= putchar(buf, c, forced);
      break;
    }

    if (success == 0) {
      buf_rollback(buf);
      return 0;
    }
  }

  return 1;
}

int
buf_appendf(struct strbuf* buf, int forced, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int success = buf_vappendf(buf, forced, fmt, ap);
    va_end(ap);
    return success;
}

void 
buf_consprint(const struct strbuf* buf) {
    if (buf->begin <= buf->end) {
        for (int i = buf->begin; i < buf->end; i++)
            consputc(buf->buf[i]);
    } else {
        for (int i = 0; i < buf->end; i++)
            consputc(buf->buf[i]);
        for (int i = buf->begin; i < buf->capacity; i++)
            consputc(buf->buf[i]);
    }
}
