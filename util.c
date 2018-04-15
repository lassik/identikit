#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

extern void usage(const char *msg) {
  fprintf(stderr, "usage: %s %s\n", progname, msg);
  exit(1);
}

extern void die(const char *msg) {
  fprintf(stderr, "%s: %s\n", progname, msg);
  exit(1);
}

extern void diesys(const char *msg) {
  fprintf(stderr, "%s: %s: %s\n", progname, msg, strerror(errno));
  exit(1);
}
