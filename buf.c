#include <sys/types.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"

static char errmsg[64];
static char *buf;
static char *pos;
static char *fill;
static char *capacity;

extern void buf_init(char *newbuf, size_t nbyte) {
  buf = pos = fill = capacity = newbuf;
  capacity += nbyte;
}

extern void buf_init_str(char *str) {
  if (!str)
    str = "";
  buf_init(str, strlen(str));
  fill = capacity;
}

extern int buf_end(void) { return pos >= fill; }

extern size_t buf_getpos(void) { return pos - buf; }

extern void buf_setpos(size_t newpos) {
  if (newpos > (size_t)(capacity - buf)) {
    exit(11);
  }
  pos = fill = buf + newpos;
}

extern void buf_puts(const char *newstr) {
  size_t newlen;

  newlen = strlen(newstr);
  if (newlen > (size_t)(capacity - pos))
    exit(12);
  memcpy(pos, newstr, newlen);
  pos += newlen;
}

extern void buf_put_dec(unsigned long val) {
  const char digits[] = "0123456789";
  char b[16];
  char *p;

  p = b + sizeof(b);
  *--p = 0;
  do {
    *--p = digits[val % strlen(digits)];
  } while (val /= strlen(digits));
  buf_puts(p);
}

extern void buf_read_fd(int fd) {
  ssize_t nread;

  nread = read(fd, pos, capacity - pos);
  if (nread == (ssize_t)-1)
    exit(1);
  fill = pos + (size_t)nread;
}

extern void buf_write_stdout(void) {
  errno = 0;
  if (write(1, buf, pos - buf) != (ssize_t)(pos - buf)) {
    buf_die_sys("write to stdout");
  }
}

extern void buf_write_stderr(void) { write(2, buf, pos - buf); }

extern void buf_skip_line(void) {
  while (pos < fill) {
    if (*pos++ == '\n')
      break;
  }
}

extern const char *buf_want_char(const char *chars) {
  const char *match;

  if (pos >= fill)
    return 0;
  if (!(match = strchr(chars, *pos)))
    return 0;
  pos++;
  return match;
}

extern void buf_need_char(const char *chars) {
  if (!buf_want_char(chars))
    exit(111);
}

static void buf_need_uint(size_t *out, const char *digits) {
  const char *digit;
  size_t value, ndigit;

  value = ndigit = 0;
  for (;;) {
    if (!(digit = buf_want_char(digits)))
      break;
    value *= strlen(digits);
    value += digit - digits;
    ndigit++;
    if (ndigit > 8)
      exit(121);
  }
  if (!ndigit)
    exit(122);
  *out = value;
}

extern void buf_need_dec(size_t *out) { buf_need_uint(out, "0123456789"); }

extern void buf_need_hex(size_t *out) {
  buf_need_uint(out, "0123456789ABCDEF");
}

extern void buf_want_spaces(void) {
  while (buf_want_char(" \t"))
    ;
}

extern void buf_need_spaces(void) {
  buf_need_char(" \t");
  buf_want_spaces();
}

extern void buf_die(const char *msg) {
  buf_init(errmsg, sizeof(errmsg));
  buf_puts(msg);
  buf_puts("\n");
  buf_write_stderr();
  exit(1);
}

extern void buf_die_sys(const char *msg) {
  buf_init(errmsg, sizeof(errmsg));
  buf_puts(msg);
  buf_puts(": ");
  buf_puts(strerror(errno));
  buf_puts("\n");
  buf_write_stderr();
  exit(1);
}
