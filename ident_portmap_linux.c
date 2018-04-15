/*
** https://github.com/ptrrkssn/pidentd/blob/master/src/k_linux.c
**
** k_linux.c - Linux 0.99.13q or later kernel access functions
**
** Copyright (c) 1997-1998 Peter Eriksson <pen@lysator.liu.se>
**
** This program is free software; you can redistribute it and/or
** modify it as you wish - as long as you don't claim that you wrote
** it.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <sys/types.h>

#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

static char buf[512];
static long wanted_local_addr, wanted_remote_addr;

static void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static void diesys(const char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

static int address_matches(long local_addr, long remote_addr) {
  return 1;
  // return (local_addr == wanted_local_addr) && (remote_addr ==
  // wanted_remote_addr);
}

static void parseline(void) {
  long local_addr, remote_addr;
  long local_port, remote_port;
  long uid, ino, dummy;
  int n;

  n = sscanf(buf, "%ld: %lX:%lx %lX:%lx %lx %lX:%lX %lx:%lX %lx %ld %ld %ld",
             &dummy, &local_addr, &local_port, &remote_addr, &remote_port,
             &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &uid, &dummy,
             &ino);
  if (n < 12) {
    return;
  }
  if (n >= 14 && !uid && !ino) {
    /*
     * Both uid and ino are zero: not even a socket?
     * Skip (continue), probably fail; or fail (break)
     * straight away? Hopefully we retry later.
     */
    return;
  }
  if (!address_matches(local_addr, remote_addr)) {
    return;
  }
  printf("%04lx%04lx%04lx\n", remote_port, local_port, uid);
}

extern int main(void) {
  FILE *fp;
  int firstline;

  // r_raddr = kp->remote.sin_addr.s_addr;
  // r_laddr = kp->local.sin_addr.s_addr;

  if (!(fp = fopen("/proc/net/tcp", "r"))) {
    diesys("open /proc/net/tcp");
  }
  firstline = 1;
  while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
    if (firstline)
      firstline = 0;
    else
      parseline();
  }
  exit(1);
}
