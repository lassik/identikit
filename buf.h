extern void buf_init(char *newbuf, size_t nbyte);
extern void buf_init_str(char *str);
extern int buf_end(void);
extern size_t buf_getpos(void);
extern void buf_setpos(size_t newpos);
extern void buf_puts(const char *newstr);
extern void buf_put_dec(unsigned long val);
extern void buf_read_fd(int fd);
extern void buf_write_stdout(void);
extern void buf_write_stderr(void);
extern void buf_need_char(const char *chars);
extern void buf_need_dec(size_t *out);
extern void buf_need_hex(size_t *out);
extern void buf_want_spaces(void);
extern void buf_need_spaces(void);
extern void buf_skip_line(void);
extern void buf_die(const char *msg);
extern void buf_die_sys(const char *msg);