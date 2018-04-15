#include <sys/types.h>

#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <arpa/inet.h>
#include <netinet/tcp_var.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "ident_portmap.h"
#include "util.h"

static void ipv6_hex(const unsigned char addr[16], char out[33]) {
  const char hex[] = "0123456789ABCDEF";
  size_t i;

  for (i = 0; i < 16; i++) {
    out[2 * i + 0] = hex[addr[i] / 16];
    out[2 * i + 1] = hex[addr[i] % 16];
  }
  out[32] = 0;
}

extern void ident_portmap_grovel(void) {
  static const char mibvar[] = "net.inet.tcp.pcblist";
  struct xinpgen *xigs;
  struct xinpgen *xig;
  struct xsocket *so;
  struct inpcb *inp;
  size_t len;
  char sbuf[33];
  char cbuf[33];

  if (saddr4 && caddr4) {
    fprintf(stderr, "wanted server ipv4 = %08X:%u\n", saddr4, sport);
    fprintf(stderr, "wanted client ipv4 = %08X:%u\n", caddr4, cport);
  } else if (!saddr4 && !caddr4) {
    ipv6_hex((unsigned char *)saddr6, sbuf);
    ipv6_hex((unsigned char *)caddr6, cbuf);
    fprintf(stderr, "wanted server ipv6 = %s:%u\n", sbuf, sport);
    fprintf(stderr, "wanted client ipv6 = %s:%u\n", cbuf, cport);
  } else {
    errno = 0;
    diesys("some mix of ipv4 and ipv6? wtf?");
  }

  if (sysctlbyname(mibvar, 0, &len, 0, 0) == -1) {
    diesys("sysctl");
  }
  if (!(xig = xigs = malloc(len))) {
    diesys("malloc");
  }
  if (sysctlbyname(mibvar, xigs, &len, 0, 0) == -1) {
    free(xigs);
    diesys("sysctl");
  }
#if 0
  if (setregid(NOBODY_GID, NOBODY_GID) == -1) {
    diesys("helper: cannot change to unprivileged group");
  }
  if (setreuid(NOBODY_UID, NOBODY_UID) == -1) {
    diesys("helper: cannot change to unprivileged user");
  }
#endif
  for (;;) {
    xig = (struct xinpgen *)((char *)xig + xig->xig_len);
    if (xig->xig_len <= sizeof(struct xinpgen)) {
      break;
    }
    so = &((struct xtcpcb *)xig)->xt_socket;
    inp = &((struct xtcpcb *)xig)->xt_inp;
    if (so->xso_protocol != IPPROTO_TCP) {
      continue;
    }
    if (inp->inp_gencnt > xigs->xig_gen) {
      continue;
    }
    if (inet_lnaof(inp->inp_laddr) == INADDR_ANY) {
      continue;
    }

    if (inp->inp_vflag & INP_IPV4) {
      fprintf(stderr, "TCP/IPv4 %08X:%-5u %08X:%u %u\n", inp->inp_laddr.s_addr,
              ntohs(inp->inp_lport), inp->inp_faddr.s_addr,
              ntohs(inp->inp_fport), so->so_uid);
    } else if (inp->inp_vflag & INP_IPV6) {
      ipv6_hex((unsigned char *)&inp->in6p_laddr, cbuf);
      ipv6_hex((unsigned char *)&inp->in6p_faddr, sbuf);
      fprintf(stderr, "TCP/IPv6 %s:%-5u %s:%u %u\n", cbuf,
              ntohs(inp->inp_lport), sbuf, ntohs(inp->inp_fport), so->so_uid);
    } else {
      continue;
    }

    // (inp->inp_flags & INP_ANONPORT)
    if (cport != ntohs(inp->inp_lport)) {
      continue;
    }
    if (sport != ntohs(inp->inp_fport)) {
      continue;
    }

    if ((inp->inp_vflag & INP_IPV4) && saddr4 && caddr4) {
      if (caddr4 != inp->inp_laddr.s_addr) {
        continue;
      }
      if (saddr4 != inp->inp_faddr.s_addr) {
        continue;
      }
    } else if ((inp->inp_vflag & INP_IPV6) && !saddr4 && !caddr4) {
      if (memcmp(caddr6, &inp->in6p_laddr, 16)) {
        continue;
      }
      if (memcmp(saddr6, &inp->in6p_faddr, 16)) {
        continue;
      }
    } else {
      continue;
    }

    found_uid = so->so_uid;
    break;
  }
  free(xigs);
}
