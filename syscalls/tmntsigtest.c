#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"

#define BUFSIZE 1024

static uint8_t charbuf;
static volatile uint8_t* badbuf = 0;
void segfault_sighandler (int signum);
void alarm_sighandler (int signum);

int main ()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    if (0 != tmnt_getargs (buf, BUFSIZE)) {
        tmnt_fdputs (1, (uint8_t*)"could not read argument\n");
	return 3;
    }

	if (buf[0] == '1') {
		tmnt_fdputs(1, (uint8_t*)"Installing signal handlers\n");
		tmnt_set_handler(SEGFAULT, segfault_sighandler);
		tmnt_set_handler(ALARM, alarm_sighandler);
	}

    tmnt_fdputs (1, (uint8_t*)"Hi, what's your name? ");
    if (-1 == (cnt = tmnt_read (0, buf, BUFSIZE-1))) {
        tmnt_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
    return 3;
    }

    (*badbuf) = 1;
    buf[cnt] = '\0';
    tmnt_fdputs (1, (uint8_t*)"Hello, ");
    tmnt_fdputs (1, buf);
	if (charbuf == 1) {
		tmnt_fdputs(1, (uint8_t*)"success\n");
	} else {
		tmnt_fdputs(1, (uint8_t*)"failure\n");
	}

    return 0;
}

void
segfault_sighandler (int signum)
{
    char buf;
	uint32_t* eax;
    tmnt_fdputs(1, (uint8_t*)"Segfault signal handler called, signum: ");
    switch (signum) {
        case 0: tmnt_fdputs(1, (uint8_t*)"0\n"); break;
        case 1: tmnt_fdputs(1, (uint8_t*)"1\n"); break;
        case 2: tmnt_fdputs(1, (uint8_t*)"2\n"); break;
        case 3: tmnt_fdputs(1, (uint8_t*)"3\n"); break;
        default: tmnt_fdputs(1, (uint8_t*)"invalid\n"); break;
    }
    tmnt_fdputs(1, (uint8_t*)"Press enter to continue...\n");
    tmnt_read(0, &buf, 1);
	badbuf = &charbuf;
	eax = (uint32_t*)(&signum + 7);
	*eax = (uint32_t)&charbuf;

    tmnt_fdputs(1, (uint8_t*)"Signal handler returning\n");
}

void
alarm_sighandler (int signum)
{
    tmnt_fdputs(1, (uint8_t*)"Alarm signal handler called, signum: ");
    switch (signum) {
        case 0: tmnt_fdputs(1, (uint8_t*)"0\n"); break;
        case 1: tmnt_fdputs(1, (uint8_t*)"1\n"); break;
        case 2: tmnt_fdputs(1, (uint8_t*)"2\n"); break;
        case 3: tmnt_fdputs(1, (uint8_t*)"3\n"); break;
        default: tmnt_fdputs(1, (uint8_t*)"invalid\n"); break;
    }
}
