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
#include "util.h"

#include "ident_portmap.h"

typedef int (*fun_t)(int socket, struct sockaddr *restrict address,
                     socklen_t *restrict address_len);

const char progname[] = "ident-portmap";

uint32_t saddr4;
uint32_t caddr4;
unsigned char saddr6[16];
unsigned char caddr6[16];

static void ipv6_hex(const unsigned char addr[16], char out[33]) {
  const char hex[] = "0123456789ABCDEF";
  size_t i;

  for (i = 0; i < 16; i++) {
    out[2 * i + 0] = hex[addr[i] / 16];
    out[2 * i + 1] = hex[addr[i] % 16];
  }
  out[32] = 0;
}

static void get_ip_address(const char *funname, fun_t fun, uint32_t *out4,
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

extern int main(int argc, char **argv) {
  char shex[33];
  char chex[33];

  if (argc < 2) {
    usage("prog");
  }
  get_ip_address("getsockname", getsockname, &saddr4, saddr6);
  get_ip_address("getpeername", getpeername, &caddr4, caddr6);
  if (saddr4 && caddr4) {
    fprintf(stderr, "4:%08X,%08X\n", saddr4, caddr4);
  } else if (!saddr4 && !caddr4) {
    ipv6_hex((unsigned char *)saddr6, shex);
    ipv6_hex((unsigned char *)caddr6, chex);
    fprintf(stderr, "6:%s,%s\n", shex, chex);
  } else {
    die("some mix of ipv4 and ipv6? wtf?");
  }
  ident_portmap_grovel();
  execvp(argv[1], argv + 1);
  buf_die_sys("exec");
}
