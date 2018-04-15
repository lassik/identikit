#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ident_portmap.h"
#include "util.h"

extern void ident_portmap_grovel(void) {
  static char buf[128];
  unsigned long xcaddr4, cport, xsaddr4, sport, uid, x;
  int firstline, n;
  FILE *stream;

  if (!caddr4 || !saddr4) {
    return;
  }
  if (!(stream = fopen("/proc/net/tcp", "r"))) {
    diesys("open /proc/net/tcp");
  }
  firstline = 1;
  while (fgets(buf, sizeof(buf), stream)) {
    if (firstline) {
      firstline = 0;
      continue;
    }
    n = sscanf(buf, "%lu: %lX:%lx %lX:%lx %lx %lX:%lX %lx:%lX %lx %lu %lu %lu",
               &x, &xcaddr4, &cport, &xsaddr4, &sport, &x, &x, &x, &x, &x, &x,
               &uid, &x, &x);
    if ((n < 12) || (xcaddr4 != caddr4) || (xsaddr4 != saddr4)) {
      continue;
    }
    yield_uid(sport, cport, uid);
  }
}
