#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char unisig[38] =
    "\xffUnisig\x00\x0a\x0d\x0a\x1aio.lassi.identikit.usermap";

static char data[4096 * 4];
static size_t datasize;

static void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static char *get_ident_for_uid(const char *uid_envar) {
  static char key[24];
  const char *uid_str;
  char *match;
  size_t keysize;

  if (!(uid_str = getenv(uid_envar))) {
    return 0;
  }
  if ((keysize = strlen(uid_str) + 4) > sizeof(key)) {
    return 0;
  }
  key[1] = 'U';
  memcpy(key + 2, uid_str, strlen(uid_str));
  key[keysize - 1] = 'I';
  if ((datasize < sizeof(unisig)) || memcmp(data, unisig, sizeof(unisig))) {
    fprintf(stderr, "warning: corrupt\n");
    return 0;
  }
  if (!(match = memmem(data + sizeof(unisig), datasize, key, keysize))) {
    return 0;
  }
  match += keysize;
  if (!memchr(match, 0, datasize - (match - data))) {
    fprintf(stderr, "warning: corrupt\n");
    return 0;
  }
  return match;
}

extern int main(int argc, char **argv) {
  int fd;
  ssize_t nr;
  char *mapfile;
  char *ident;

  if (argc < 3) {
    die("usage");
  }
  mapfile = argv[1];
  argv += 2;
  if ((fd = open(mapfile, O_RDONLY | O_NOFOLLOW)) == -1) {
    fprintf(stderr, "warning: cannot open %s: %s\n", mapfile, strerror(errno));
    exit(1);
  }
  if ((nr = read(fd, data, sizeof(data))) == (ssize_t)-1) {
    fprintf(stderr, "warning: cannot read %s: %s\n", mapfile, strerror(errno));
    exit(1);
  }
  datasize = nr;
  ident = get_ident_for_uid("IDENT_USER");
  if (!ident) {
    setenv("IDENT_ERROR", "NO-USER", 1);
    ident = "";
  } else if (!ident[0]) {
    setenv("IDENT_ERROR", "HIDDEN-USER", 1);
  }
  setenv("IDENT_USER", ident, 1);
  execvp(argv[0], argv);
  fprintf(stderr, "cannot exec\n");
  exit(1);
}
