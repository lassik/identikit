#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char *env_search(const char *envar, int letter, const char *key) {
  static char buf[32];
  const char *match;
  const char *limit;
  size_t len;

  if (strchr(key, ':')) {
    return 0;
  }
  if ((size_t)snprintf(buf, sizeof(buf), ":%c%s:", letter, key) >=
      sizeof(buf)) {
    return 0;
  }
  if (!(match = getenv(envar))) {
    return 0;
  }
  if (!(match = strstr(match, buf))) {
    return 0;
  }
  match += strlen(buf);
  if (!(limit = strchr(match, ':'))) {
    return 0;
  }
  len = limit - match;
  if (len >= sizeof(buf)) {
    return 0;
  }
  memcpy(buf, match, len);
  buf[len] = 0;
  return buf;
}
