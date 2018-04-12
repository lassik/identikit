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

#include "ident_read.h"

static void diesys(const char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

extern void ident_read_grovel(void) {
  static const char mibvar[] = "net.inet.tcp.pcblist";
  struct xinpgen *xigs;
  struct xinpgen *xig;
  struct inpcb *inp;
  struct xsocket *so;
  uid_t uid;
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
  if (setregid(NOBODY_GID, NOBODY_GID) == -1) {
    diesys("helper: cannot change to unprivileged group");
  }
  if (setreuid(NOBODY_UID, NOBODY_UID) == -1) {
    diesys("helper: cannot change to unprivileged user");
  }
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

    // (inp->inp_flags & INP_ANONPORT)
    if (remoteport != ntohs(inp->inp_fport)) {
      continue;
    }
    if (localport != ntohs(inp->inp_lport)) {
      continue;
    }

    if ((inp->inp_vflag & INP_IPV4) && local4 && remote4) {
      if (local4 != inp->inp_laddr.s_addr) {
        continue;
      }
      if (remote4 != inp->inp_faddr.s_addr) {
        continue;
      }
    } else if ((inp->inp_vflag & INP_IPV6) && !local4 && !remote4) {
      if (memcmp(local6, &inp->in6p_laddr, 16)) {
        continue;
      }
      if (memcmp(remote6, &inp->in6p_faddr, 16)) {
        continue;
      }
    } else {
      continue;
    }

    uid = so->so_uid;
  }
  free(xigs);
}
