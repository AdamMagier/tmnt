#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];
    tmnt_fdputs (1, (uint8_t*)"Starting TMNT Shell\n");

    while (1) {
        tmnt_fdputs (1, (uint8_t*)"TMNT> ");
	if (-1 == (cnt = tmnt_read (0, buf, BUFSIZE-1))) {
	    tmnt_fdputs (1, (uint8_t*)"read from keyboard failed\n");
	    return 3;
	}
	if (cnt > 0 && '\n' == buf[cnt - 1])
	    cnt--;
	buf[cnt] = '\0';
	if (0 == tmnt_strcmp (buf, (uint8_t*)"exit"))
	    return 0;
	if ('\0' == buf[0])
	    continue;
	rval = tmnt_execute (buf);
	if (-1 == rval)
	    tmnt_fdputs (1, (uint8_t*)"no such command\n");
	else if (256 == rval)
	    tmnt_fdputs (1, (uint8_t*)"program terminated by exception\n");
	else if (0 != rval)
	    tmnt_fdputs (1, (uint8_t*)"program terminated abnormally\n");
    }
}

