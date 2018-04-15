#include <string.h>

#include "env_write.h"
#include "util.h"

char env_write_buf[16 * 1024] = ":";
static size_t env_write_len = 1;

extern void env_write(int letter, const void *str) {
  size_t len;

  if (strchr(str, ':')) {
    return;
  }
  len = strlen(str);
  if (sizeof(env_write_buf) - env_write_len < len + 3) {
    die("out of space for environment variable");
  }
  env_write_buf[env_write_len++] = letter;
  memcpy(env_write_buf + env_write_len, str, len);
  env_write_len += len;
  env_write_buf[env_write_len++] = ':';
  env_write_buf[env_write_len++] = 0;
}
