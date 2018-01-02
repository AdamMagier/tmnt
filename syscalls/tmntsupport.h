#if !defined(TMNTSUPPORT_H)
#define TMNTSUPPORT_H

extern uint32_t tmnt_strlen(const uint8_t* s);
extern void tmnt_strcpy(uint8_t* dst, const uint8_t* src);
extern void tmnt_fdputs(int32_t fd, const uint8_t* s);
extern int32_t tmnt_strcmp(const uint8_t* s1, const uint8_t* s2);
extern int32_t tmnt_strncmp(const uint8_t* s1, const uint8_t* s2, uint32_t n);
extern uint8_t *tmnt_itoa(uint32_t value, uint8_t* buf, int32_t radix);
extern uint8_t *tmnt_strrev(uint8_t* s);

#endif /* TMNTSUPPORT_H */

