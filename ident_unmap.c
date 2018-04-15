#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "env_search.h"
#include "util.h"

const char progname[] = "ident-unmap";

static void portmap(void) {
  static char ports[24];
  const char *sport;
  const char *cport;
  const char *value;

  if (!(sport = getenv("IDENT_SERVERPORT"))) {
    return;
  }
  if (!(cport = getenv("IDENT_CLIENTPORT"))) {
    return;
  }
  if ((size_t)snprintf(ports, sizeof(ports), "%s,%s", sport, cport) >=
      sizeof(ports)) {
    return;
  }
  if (!(value = env_search("IDENT_PORTMAP", 'P', ports))) {
    return;
  }
  switch (value[0]) {
  case 'S':
    setenv("IDENT_SERVER", &value[1], 1);
    break;
  case 'U':
    setenv("IDENT_USER", &value[1], 1);
    break;
  }
}

static void usermap(void) {
  const char *user;

  if (!(user = getenv("IDENT_USER"))) {
    setenv("IDENT_USER", "", 1);
    setenv("IDENT_ERROR", "NO-USER", 1);
    return;
  }
  if (!(user = env_search("IDENT_USERMAP", 'U', user))) {
    setenv("IDENT_ERROR", "NO-USER", 1);
    return;
  }
  if (user[0] != 'I') {
    setenv("IDENT_ERROR", "NO-USER", 1);
    return;
  }
  user++;
  if (!user[0]) {
    setenv("IDENT_ERROR", "HIDDEN-USER", 1);
    return;
  }
  setenv("IDENT_USER", user, 1);
}

extern int main(int argc, char **argv) {
  if (argc < 2) {
    usage("prog");
  }
  portmap();
  usermap();
  execvp(argv[1], &argv[1]);
  diesys("exec");
}
