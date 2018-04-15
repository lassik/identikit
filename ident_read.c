#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"
#include "util.h"

const char progname[] = "ident-read";

extern int main(int argc, char **argv) {
  static char buffer[32];
  static char sportstr[8];
  static char cportstr[8];
  size_t sport;
  size_t cport;

  if (argc < 2) {
    usage("prog");
  }
  buf_init(buffer, sizeof(buffer));
  buf_read_fd(0);
  buf_need_dec(&sport);
  buf_want_spaces();
  buf_need_char(",");
  buf_want_spaces();
  buf_need_dec(&cport);
  snprintf(sportstr, sizeof(sportstr), "%zu", sport);
  snprintf(cportstr, sizeof(cportstr), "%zu", cport);
  setenv("IDENT_SERVERPORT", sportstr, 1);
  setenv("IDENT_CLIENTPORT", cportstr, 1);
  execvp(argv[1], &argv[1]);
  buf_die_sys("exec");
}
