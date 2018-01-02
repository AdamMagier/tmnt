#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    tmnt_fdputs (1, (uint8_t*)"Hi, what's your name? ");
    if (-1 == (cnt = tmnt_read (0, buf, BUFSIZE-1))) {
        tmnt_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
        return 3;
    }
    buf[cnt] = '\0';
    tmnt_fdputs (1, (uint8_t*)"Hello, ");
    tmnt_fdputs (1, buf);

    return 0;
}

