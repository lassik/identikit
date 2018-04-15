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
#include "env_write.h"
#include "util.h"

#include "ident_portmap.h"

uint32_t saddr4;
uint32_t caddr4;
unsigned char saddr6[16];
unsigned char caddr6[16];
static char shex[33];
static char chex[33];

const char progname[] = "ident-portmap";

extern void yield_uid(unsigned int sport, unsigned int cport, uid_t uid) {
  static char buf[24];

  snprintf(buf, sizeof(buf), "%u,%u", sport, cport);
  env_write('P', buf);
  snprintf(buf, sizeof(buf), "%u", uid);
  env_write('U', buf);
}

static void ipv6_hex(const unsigned char addr[16], char out[33]) {
  const char hex[] = "0123456789ABCDEF";
  size_t i;

  for (i = 0; i < 16; i++) {
    out[2 * i + 0] = hex[addr[i] / 16];
    out[2 * i + 1] = hex[addr[i] % 16];
  }
  out[32] = 0;
}

typedef int (*fun_t)(int socket, struct sockaddr *restrict address,
                     socklen_t *restrict address_len);

static void get_ip_address(const char *funname, fun_t fun, uint32_t *out4,
                           unsigned char out6[16], char outhex[33]) {
  static struct sockaddr_storage addr;
  socklen_t socklen;

  socklen = sizeof(addr);
  if (fun(0, (struct sockaddr *)&addr, &socklen) == -1) {
    diesys(funname);
  }
  switch (addr.ss_family) {
  case AF_INET:
    *out4 = ((struct sockaddr_in *)&addr)->sin_addr.s_addr;
    snprintf(outhex, 33, "%08X", ntohl(*out4));
    break;
  case AF_INET6:
    memcpy(out6, &((struct sockaddr_in6 *)&addr)->sin6_addr, 16);
    ipv6_hex(out6, outhex);
    break;
  default:
    die("not an IP socket");
  }
}

extern int main(int argc, char **argv) {
  if (argc < 2) {
    usage("prog");
  }
  get_ip_address("getsockname", getsockname, &saddr4, saddr6, shex);
  get_ip_address("getpeername", getpeername, &caddr4, caddr6, chex);
  if (!!saddr4 != !!caddr4) {
    die("server and client are mixed IPv4 and IPv6, huh?");
  }
  setenv("IDENT_SERVER", shex, 1);
  setenv("IDENT_CLIENT", chex, 1);
  ident_portmap_grovel();
  setenv("IDENT_PORTMAP", env_write_buf, 1);
  execvp(argv[1], &argv[1]);
  diesys("exec");
}
