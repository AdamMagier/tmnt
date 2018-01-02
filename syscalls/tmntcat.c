#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[1024];

    if (0 != tmnt_getargs (buf, 1024)) {
        tmnt_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }

    if (-1 == (fd = tmnt_open (buf))) {
        tmnt_fdputs (1, (uint8_t*)"file not found\n");
	return 2;
    }

    while (0 != (cnt = tmnt_read (fd, buf, 1024))) {
        if (-1 == cnt) {
	    tmnt_fdputs (1, (uint8_t*)"file read failed\n");
	    return 3;
	}
	if (-1 == tmnt_write (1, buf, cnt))
	    return 3;
    }

    return 0;
}

