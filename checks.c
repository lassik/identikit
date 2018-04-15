static int is_allowed_cport(void) {
  size_t port;

  return 1;
  buf_init_str(getenv("IDENT_ALLOWPORTS"));
  while (!buf_end()) {
    buf_need_dec(&port);
    if (cport == port) {
      return 1;
    }
  }
  return 0;
  if (cport < 1024) {
    buf_die("ident client is coming from a privileged port, what's going on?");
  }
  if (!is_allowed_cport()) {
    buf_die("port is not in the list of allowed remote ports");
  }
  // static int is_safe_uid(uid_t uid) { return (uid > 500) && (uid < 65534); }
}

void() {
  unsigned int sport;
  unsigned int cport;
  uid_t found_uid;
  static char sportstr[8];
  static char cportstr[8];
  static char uidstr[8];
  if (found_uid) {
    snprintf(uidstr, sizeof(uidstr), "%lu", (unsigned long)found_uid);
  } else {
    uidstr[0] = 0;
    setenv("IDENT_ERROR", "NO-USER", 1);
    fprintf(stderr, "uid not found\n");
  }
  setenv("IDENT_USER", uidstr, 1);
}

void() {
  if (inp->inp_vflag & INP_IPV4) {
    fprintf(stderr, "TCP/IPv4 %08X:%-5u %08X:%u %u\n", inp->inp_laddr.s_addr,
            ntohs(inp->inp_lport), inp->inp_faddr.s_addr, ntohs(inp->inp_fport),
            so->so_uid);
  } else if (inp->inp_vflag & INP_IPV6) {
    ipv6_hex((unsigned char *)&inp->in6p_laddr, cbuf);
    ipv6_hex((unsigned char *)&inp->in6p_faddr, sbuf);
    fprintf(stderr, "TCP/IPv6 %s:%-5u %s:%u %u\n", cbuf, ntohs(inp->inp_lport),
            sbuf, ntohs(inp->inp_fport), so->so_uid);
  } else {
    continue;
  }
}

#if 0
  if (setregid(NOBODY_GID, NOBODY_GID) == -1) {
    diesys("helper: cannot change to unprivileged group");
  }
  if (setreuid(NOBODY_UID, NOBODY_UID) == -1) {
    diesys("helper: cannot change to unprivileged user");
  }
#endif
