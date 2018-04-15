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

extern void ident_portmap_grovel(void) {
  static const char mibvar[] = "net.inet.tcp.pcblist";
  struct xinpgen *xigs;
  struct xinpgen *xig;
  struct xtcpcb *xt;
  struct xsocket *so;
  struct inpcb *inp;
  size_t len;

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
  for (;;) {
    xig = (struct xinpgen *)((char *)xig + xig->xig_len);
    if (xig->xig_len <= sizeof(struct xinpgen)) {
      break;
    }
    xt = (struct xtcpcb *)xig;
    so = &xt->xt_socket;
    inp = &xt->xt_inp;
    if (so->xso_protocol != IPPROTO_TCP) {
      continue;
    }
    if (inp->inp_gencnt > xigs->xig_gen) {
      continue;
    }
    if ((inp->inp_vflag & INP_IPV4) && saddr4 && caddr4) {
      if (saddr4 != inp->inp_faddr.s_addr) {
        continue;
      }
      if (caddr4 != inp->inp_laddr.s_addr) {
        continue;
      }
    } else if ((inp->inp_vflag & INP_IPV6) && !saddr4 && !caddr4) {
      if (memcmp(saddr6, &inp->in6p_faddr, 16)) {
        continue;
      }
      if (memcmp(caddr6, &inp->in6p_laddr, 16)) {
        continue;
      }
    } else {
      continue;
    }
    yield_uid(ntohs(inp->inp_fport), ntohs(inp->inp_lport), so->so_uid);
  }
  free(xigs);
}
