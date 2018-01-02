#include <stdint.h>

#include "tmntsupport.h"
#include "tmntsyscall.h"

int main ()
{

    tmnt_fdputs (1, (uint8_t*)"Hello, if this ran, the program was correct. Yay!\n");

    return 0;
}

