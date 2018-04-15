#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

const char progname[] = "ident-user";

static const char unisig[38] =
    "\xffUnisig\x00\x0a\x0d\x0a\x1aio.lassi.identikit.usermap";

static char *get_ident_for_uid(const char *mapfile, const char *uid_str) {
  static char file[4096 * 4];
  static size_t filesize;
  static char key[24];
  size_t keysize;
  char *match;
  int fd;

  if ((fd = open(mapfile, O_RDONLY | O_NOFOLLOW)) == -1) {
    fprintf(stderr, "warning: cannot open usermap file: %s\n", strerror(errno));
    setenv("IDENT_ERROR", "UNKNOWN-ERROR", 1);
    return 0;
  }
  if ((filesize = read(fd, file, sizeof(file))) == (size_t)-1) {
    fprintf(stderr, "warning: cannot read usermap file: %s\n", strerror(errno));
    setenv("IDENT_ERROR", "UNKNOWN-ERROR", 1);
    return 0;
  }
  close(fd);
  if ((keysize = strlen(uid_str) + 4) > sizeof(key)) {
    setenv("IDENT_ERROR", "NO-USER", 1);
    return 0;
  }
  key[1] = 'U';
  memcpy(key + 2, uid_str, strlen(uid_str));
  key[keysize - 1] = 'I';
  if ((filesize < sizeof(unisig)) || memcmp(file, unisig, sizeof(unisig))) {
    goto corrupt;
  }
  if (!(match = memmem(file + sizeof(unisig), filesize, key, keysize))) {
    setenv("IDENT_ERROR", "NO-USER", 1);
    return 0;
  }
  match += keysize;
  if (!memchr(match, 0, filesize - (match - file))) {
    goto corrupt;
  }
  return match;
corrupt:
  fprintf(stderr, "warning: corrupt usermap file\n");
  return 0;
}

extern int main(int argc, char **argv) {
  char *uid_str;
  char *ident;

  if (argc < 3) {
    fprintf(stderr, "usage\n");
    exit(1);
  }
  if (!(uid_str = getenv("IDENT_USER"))) {
    return 0;
  }
  if (!(ident = get_ident_for_uid(argv[1], uid_str))) {
    ident = "";
  } else if (!ident[0]) {
    setenv("IDENT_ERROR", "HIDDEN-USER", 1);
  }
  setenv("IDENT_USER", ident, 1);
  execvp(argv[2], &argv[2]);
  fprintf(stderr, "cannot exec\n");
  exit(1);
}
