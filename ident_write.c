#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buf.h"

static char buffer[64];

extern int main(void) {
  char *str;
  char *user;
  char *syst;
  char *erro;
  size_t localport;
  size_t remoteport;

  buf_init_str(getenv("IDENT_LOCALPORT"));
  buf_need_dec(&localport);

  buf_init_str(getenv("IDENT_REMOTEPORT"));
  buf_need_dec(&remoteport);

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
  buf_put_dec(localport);
  buf_puts(",");
  buf_put_dec(remoteport);
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
  buf_write_stdout();
  return 0;
}
