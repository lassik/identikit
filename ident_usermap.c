#include <sys/types.h>

#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "env_write.h"
#include "util.h"

enum { MAXENTRIES = 512, MAXIDENTLEN = 31 };

struct entry {
  char *actual_username; /* Not written to output file */
  char *ident;
  char uidstr[24];
};

const char progname[] = "ident-usermap";

static struct entry entries[MAXENTRIES];
static size_t nentries;
static char actual_usernames[4096]; /* \0 alice \0 bob \0 */
static size_t actual_usernames_len = 1;

static void diemem(void) { die("out of memory"); }

static void add_to_actual_usernames(const char *username) {
  size_t size;

  size = strlen(username) + 1;
  if (size > sizeof(actual_usernames) - actual_usernames_len) {
    diemem();
  }
  memcpy(actual_usernames + actual_usernames_len, username, size);
  actual_usernames_len += size;
}

static int is_actual_username(const char *username) {
  char buf[strlen(username) + 2];
  memset(buf, 0, sizeof(buf));
  memcpy(buf + 1, username, strlen(username));
  return !!memmem(actual_usernames, actual_usernames_len, buf, sizeof(buf));
}

static int is_impostor(struct entry *entry) {
  return strcmp(entry->actual_username, entry->ident) &&
         is_actual_username(entry->ident);
}

static void deal_with_impostors(void) {
  struct entry *entry;

  for (entry = entries; entry < entries + nentries; entry++) {
    if (is_impostor(entry)) {
      fprintf(stderr, "warning: changing impostor %s to %s\n", entry->ident,
              entry->actual_username);
      entry->ident = entry->actual_username;
    }
  }
}

static char *ident_from_dir(const char *dir) {
  static char buf[MAXIDENTLEN + 1];
  static struct stat st;
  char *ident;
  ssize_t nr, i;
  int fd;

  if (chdir(dir) == -1) {
    fprintf(stderr, "warning: cannot chdir to user home %s\n", dir);
    return 0;
  }
  if (lstat(".noident", &st) != -1) {
    return "";
  } else if ((fd = open(".fakeid", O_RDONLY | O_NOFOLLOW)) != -1) {
    memset(buf, 0, sizeof(buf));
    nr = read(fd, buf, MAXIDENTLEN);
    close(fd);
    if (nr == (ssize_t)-1) {
      fprintf(stderr, "warning: .fakeid found but unreadable %s\n", dir);
      return 0;
    }
    for (i = 0; i < nr; i++) {
      if (isspace(buf[i])) {
        break;
      }
      if (!isprint(buf[i])) {
        buf[i] = '_';
      }
    }
    buf[i] = 0;
    if (i < 1) {
      fprintf(stderr, "warning: .fakeid empty %s\n", dir);
      return 0;
    }
    if (!(ident = strdup(buf))) {
      diemem();
    }
    return ident;
  }
  return 0;
}

static void add_entry_from_pw(struct entry *entry, struct passwd *pw) {
  snprintf(entry->uidstr, sizeof(entry->uidstr), "%llu",
           (unsigned long long)pw->pw_uid);
  entry->actual_username = strdup(pw->pw_name);
  if (!(entry->ident = ident_from_dir(pw->pw_dir))) {
    entry->ident = entry->actual_username;
  }
}

static void fill_envbuf(void) {
  struct entry *entry;

  for (entry = entries; entry < entries + nentries; entry++) {
    env_write('U', entry->uidstr);
    env_write('I', entry->ident);
  }
}

extern int main(int argc, char **argv) {
  struct passwd *pw;

  if (argc < 2) {
    usage("prog");
  }
  for (;;) {
    errno = 0;
    if (!(pw = getpwent())) {
      if (errno) {
        diesys("getpwent failed");
      }
      break;
    }
    add_to_actual_usernames(pw->pw_name);
    if ((pw->pw_uid < MIN_CLIENT_UID) || (pw->pw_uid > MAX_CLIENT_UID)) {
      continue;
    }
    if (nentries >= MAXENTRIES) {
      continue;
    }
    add_entry_from_pw(entries + nentries++, pw);
  }
  deal_with_impostors();
  fill_envbuf();
  setenv("IDENT_USERMAP", env_write_buf, 1);
  execvp(argv[1], &argv[1]);
  diesys("exec");
}
