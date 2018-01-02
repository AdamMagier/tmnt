/* sys_call_asm.S */

#include "sysnum.h"
.data

    FIRST_ARG  = 0x08
    SECOND_ARG = 0x0C
    THIRD_ARG  = 0x10
    
    SYS_CALL_BASE = 0x80
    
    POP_ONE = 0x04

#define USR_CALL(name,number)   \
.globl name                    ;\
name:                          ;\
    pushl %ebx                 ;\
    movl $number,%eax          ;\
    movl FIRST_ARG(%esp),%ebx  ;\
    movl SECOND_ARG(%esp),%ecx ;\
    movl THIRD_ARG(%esp),%edx  ;\
    int $SYS_CALL_BASE         ;\
    popl %ebx                  ;\
    ret                         \
    
#define SYS_CALL(asm_name,c_name) \
.globl asm_name                  ;\
asm_name:                        ;\
    pushl %edx                   ;\
    pushl %ecx                   ;\
    pushl %ebx                   ;\
    call c_name                  ;\
    popl %ebx                    ;\
    popl %ecx                    ;\
    popl %edx                    ;\
    ret                           \

USR_CALL(sys_halt_usr,SYS_HALT)
USR_CALL(sys_execute_usr,SYS_EXECUTE)
USR_CALL(sys_read_usr,SYS_READ)
USR_CALL(sys_write_usr,SYS_WRITE)
USR_CALL(sys_open_usr,SYS_OPEN)
USR_CALL(sys_close_usr,SYS_CLOSE)
USR_CALL(sys_getargs_usr,SYS_GETARGS)
USR_CALL(sys_vidmap_usr,SYS_VIDMAP)
USR_CALL(sys_set_handler_usr,SYS_SET_HANDLER)
USR_CALL(sys_sigreturn_usr,SYS_SIGRETURN)

SYS_CALL(sys_halt_asm,sys_halt)
SYS_CALL(sys_execute_asm,sys_execute)
SYS_CALL(sys_read_asm,sys_read)
SYS_CALL(sys_write_asm,sys_write)
SYS_CALL(sys_open_asm,sys_open)
SYS_CALL(sys_close_asm,sys_close)
SYS_CALL(sys_getargs_asm,sys_getargs)
SYS_CALL(sys_vidmap_asm,sys_vidmap)
SYS_CALL(sys_set_handler_asm,sys_set_handler)
SYS_CALL(sys_sigreturn_asm,sys_sigreturn)



#performs a context switch using iret and the data stored on the stack
.globl context_switch
context_switch:
    addl $POP_ONE, %esp
    iret

/* Might need to call start */
