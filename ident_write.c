#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"

const char progname[] = "ident-write";

static char buffer[64];

extern int main(void) {
  char *str;
  char *user;
  char *syst;
  char *erro;
  size_t serverport;
  size_t clientport;

  buf_init_str(getenv("IDENT_SERVERPORT"));
  buf_need_dec(&serverport);

  buf_init_str(getenv("IDENT_CLIENTPORT"));
  buf_need_dec(&clientport);

  str = getenv("IDENT_USER");
  if (!str)
    str = "";
  user = str;

  str = getenv("IDENT_SYSTEM");
  if (!str)
    str = "UNIX";
  syst = str;

  str = getenv("IDENT_ERROR");
  if (!str)
    str = "";
  erro = str;

  buf_init(buffer, sizeof(buffer));
  buf_put_dec(serverport);
  buf_puts(",");
  buf_put_dec(clientport);
  if (erro[0]) {
    buf_puts(":ERROR:");
    buf_puts(erro);
  } else {
    buf_puts(":USERID:");
    buf_puts(syst);
    buf_puts(":");
    buf_puts(user);
  }
  buf_puts("\n");
  fprintf(stderr, "writing response\n");
  buf_write_stdout();
  return 0;
}
