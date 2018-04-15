#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

const char progname[] = "ident-client";

static int client(void) {
  static struct sockaddr_in sin;
  static char ports[24];
  const char *sport;
  const char *cport;
  unsigned long addr;
  const char *server;
  int fd, n;
  socklen_t sinsize;

  if (!(sport = getenv("IDENT_SERVERPORT"))) {
    return 0;
  }
  if (!(cport = getenv("IDENT_CLIENTPORT"))) {
    return 0;
  }
  if ((size_t)snprintf(ports, sizeof(ports), "%s,%s\r\n", sport, cport) >=
      sizeof(ports)) {
    return 0;
  }
  if (!(server = getenv("IDENT_SERVER"))) {
    return 0;
  }
  addr = 0;
  if (strlen(server) == 8) {
    if ((n = sscanf(server, "%08lX%n", &addr, &n)) != 1) {
      return 0;
    }
  } else {
    return 0;
  }
  if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    return 0;
  }
  sin.sin_len = sinsize = sizeof(sin);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = addr;
  sin.sin_port = 113;
  if (connect(fd, (struct sockaddr *)&sin, sinsize) == -1) {
    return 0;
  }
  if (write(fd, ports, strlen(ports)) != (ssize_t)strlen(ports)) {
    return 0;
  }
  dup2(fd, 0);
  return 1;
}

extern int main(int argc, char **argv) {
  char **next_argv;

  if (argc < 2) {
    usage("readprog prog");
  }
  next_argv = &argv[1];
  if (!client()) {
    next_argv = &argv[2];
  }
  execvp(next_argv[0], next_argv);
  diesys("exec");
}
