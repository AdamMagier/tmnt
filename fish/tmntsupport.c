#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"


uint32_t
tmnt_strlen (const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void
tmnt_strcpy (uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void
tmnt_fdputs (int32_t fd, const uint8_t* s)
{
    (void)tmnt_write (fd, s, tmnt_strlen (s));
}

int32_t
tmnt_strcmp (const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
	    return 0;
	s1++;
	s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t
tmnt_strncmp (const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
	return 0;
    while (*s1 == *s2) {
        if (*s1 == '\0' || --n == 0)
	    return 0;
	s1++;
	s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

