#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char progname[] = "ident-client";

static void usage(void) {
  fprintf(stderr, "usage: %s prog\n", progname);
  exit(1);
}

static void diesys(const char *msg) {
  fprintf(stderr, "%s: %s: %s\n", progname, msg, strerror(errno));
  exit(1);
}

extern int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }
  execvp(argv[1], &argv[1]);
  diesys("exec");
}
