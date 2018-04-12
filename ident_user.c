#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const unsigned char unisig[38] =
    "\xffUnisig\x00\x0a\x0d\x0a\x1aio.lassi.identikit.usermap";

static unsigned char data[4096 * 4];
static unsigned char *ptr;
static unsigned char *limit;

static void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static void diemem(void) { die("out of memory"); }

static void dietrunc(void) {
  fprintf(stderr, "warning: truncated\n");
  exit(1);
}

static uint64_t read_vint(void) {
  uint64_t val, shift;

  val = shift = 0;
  do {
    if (ptr >= limit) {
      dietrunc();
    }
    val |= (127 & *ptr) << shift;
    shift += 7;
  } while (128 & *ptr++);
  return val;
}

static void *read_bytes(size_t nbytes) {
  void *ans;
  if ((size_t)(limit - ptr) < nbytes) {
    dietrunc();
  }
  ans = ptr;
  ptr += nbytes;
  return ans;
}

static uint64_t uint64env(const char *envar) {
  const char *str;
  unsigned long uintval;
  int count, len;

  str = getenv(envar);
  if (!str) {
    str = "";
  }
  count = sscanf(str, "%lu%n", &uintval, &len);
  if ((count != 1) || ((size_t)len != strlen(str))) {
    fprintf(stderr, "cannot parse %s\n", envar);
    exit(1);
  }
  return uintval;
}

static char *get_ident_for_uid(uint64_t goal_uid) {
  if (memcmp(read_bytes(sizeof(unisig)), unisig, sizeof(unisig))) {
    fprintf(stderr, "warning: corrupt\n");
    exit(1);
  }
  while (ptr < limit) {
    uid_t uid = read_vint();
    size_t idlen = read_vint();
    char *ident = read_bytes(idlen);
    char *identmal;
    if (uid == goal_uid) {
      if (memchr(ident, 0, idlen)) {
        fprintf(stderr, "warning: corrupt\n");
        exit(1);
      }
      if (!(identmal = calloc(1, idlen + 1))) {
        diemem();
      }
      memcpy(identmal, ident, idlen);
      return identmal;
    }
  }
  return 0;
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
  ptr = limit = data;
  limit += (size_t)nr;
  ident = get_ident_for_uid(uint64env("IDENT_USER"));
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
