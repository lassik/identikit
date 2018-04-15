extern uint32_t saddr4;
extern uint32_t caddr4;
extern unsigned char saddr6[16];
extern unsigned char caddr6[16];

extern void yield_uid(unsigned int sport, unsigned int cport, uid_t uid);
extern void ident_portmap_grovel(void);
