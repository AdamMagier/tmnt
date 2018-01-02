/* sys_call_usr.h */

#include "lib.h"

extern int32_t sys_halt_usr(uint8_t status);
extern int32_t sys_execute_usr(const uint8_t* command);
extern int32_t sys_read_usr(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write_usr(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open_usr(const uint8_t* filename);
extern int32_t sys_close_usr(int32_t fd);
extern int32_t sys_getargs_usr(uint8_t buf, int32_t nbytes);
extern int32_t sys_vidmap_usr(uint8_t** screen_start);
extern int32_t sys_set_handler_usr(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn_usr(void);
