#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"

#include "ident_read.h"

typedef int (*fun_t)(int socket, struct sockaddr *restrict address,
                     socklen_t *restrict address_len);

uint32_t local4;
uint32_t remote4;
unsigned char local6[16];
unsigned char remote6[16];
size_t localport;
size_t remoteport;
uid_t found_uid;

static char localportstr[8];
static char remoteportstr[8];
static char uidstr[8];
static char buffer[32];

static const char progname[] = "ident-read";

static void usage(void) {
  fprintf(stderr, "usage: %s prog\n", progname);
  exit(1);
}

static void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static void diesys(const char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

// static int is_safe_uid(uid_t uid) { return (uid > 500) && (uid < 65534); }

static void findout(const char *funname, fun_t fun, uint32_t *out4,
                    unsigned char out6[16]) {
  static struct sockaddr_storage addr;
  socklen_t socklen;

  socklen = sizeof(addr);
  if (fun(0, (struct sockaddr *)&addr, &socklen) == -1) {
    diesys(funname);
  }
  switch (addr.ss_family) {
  case AF_INET:
    *out4 = ((struct sockaddr_in *)&addr)->sin_addr.s_addr;
    break;
  case AF_INET6:
    memcpy(out6, &((struct sockaddr_in6 *)&addr)->sin6_addr, 16);
    break;
  default:
    die("not an IP socket");
  }
}

static void read_query(void) {
  buf_init(buffer, sizeof(buffer));
  buf_read_fd(0);
  buf_need_dec(&localport);
  buf_want_spaces();
  buf_need_char(",");
  buf_want_spaces();
  buf_need_dec(&remoteport);
}

static int is_allowed_remoteport(void) {
  size_t port;

  return 1;
  buf_init_str(getenv("IDENT_ALLOWPORTS"));
  while (!buf_end()) {
    buf_need_dec(&port);
    if (remoteport == port) {
      return 1;
    }
  }
  return 0;
}

extern int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }
  read_query();
  if (localport < 1024) {
    buf_die("root-only local port requested");
  }
  if (!is_allowed_remoteport()) {
    buf_die("port is not in the list of allowed remote ports");
  }
  snprintf(localportstr, sizeof(localportstr), "%zu", localport);
  snprintf(remoteportstr, sizeof(remoteportstr), "%zu", remoteport);
  setenv("IDENT_LOCALPORT", localportstr, 1);
  setenv("IDENT_REMOTEPORT", remoteportstr, 1);

  findout("getsockname", getsockname, &local4, local6);
  findout("getpeername", getpeername, &remote4, remote6);

  ident_read_grovel();
  if (!found_uid) {
    fprintf(stderr, "uid not found\n");
    exit(66);
  }

  snprintf(uidstr, sizeof(uidstr), "%lu", (unsigned long)found_uid);
  setenv("IDENT_USER", uidstr, 1);
  execvp(argv[1], argv + 1);
  buf_die_sys("exec");
}
