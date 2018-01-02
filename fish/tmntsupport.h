#if !defined(TMNTSUPPORT_H)
#define TMNTSUPPORT_H

extern uint32_t tmnt_strlen (const uint8_t* s);
extern void tmnt_strcpy (uint8_t* dst, const uint8_t* src);
extern void tmnt_fdputs (int32_t fd, const uint8_t* s);
extern int32_t tmnt_strcmp (const uint8_t* s1, const uint8_t* s2);
extern int32_t tmnt_strncmp (const uint8_t* s1, const uint8_t* s2, uint32_t n);

#endif /* TMNTSUPPORT_H */
