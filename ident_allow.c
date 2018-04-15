#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"
#include "util.h"

const char progname[] = "ident-allow";

extern int main(int argc, char **argv) {
  if (argc < 2) {
    usage("prog");
  }
  execvp(argv[1], &argv[1]);
  diesys("exec");
}
