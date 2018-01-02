#include "tmntsysnum.h"

/* 
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */
#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

/* the system call library wrappers */
DO_CALL(tmnt_halt,SYS_HALT)
DO_CALL(tmnt_execute,SYS_EXECUTE)
DO_CALL(tmnt_read,SYS_READ)
DO_CALL(tmnt_write,SYS_WRITE)
DO_CALL(tmnt_open,SYS_OPEN)
DO_CALL(tmnt_close,SYS_CLOSE)
DO_CALL(tmnt_getargs,SYS_GETARGS)
DO_CALL(tmnt_vidmap,SYS_VIDMAP)
DO_CALL(tmnt_set_handler,SYS_SET_HANDLER)
DO_CALL(tmnt_sigreturn,SYS_SIGRETURN)


/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
    PUSHL   $0
    PUSHL   $0
	PUSHL	%EAX
	CALL	tmnt_halt

