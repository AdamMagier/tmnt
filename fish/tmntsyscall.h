#if !defined(TMNTSYSCALL_H)
#define TMNTSYSCALL_H

#include <stdint.h>

/* All calls return >= 0 on success or -1 on failure. */

/*  
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */ 
extern int32_t tmnt_halt (uint8_t status);
extern int32_t tmnt_execute (const uint8_t* command);
extern int32_t tmnt_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t tmnt_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t tmnt_open (const uint8_t* filename);
extern int32_t tmnt_close (int32_t fd);
extern int32_t tmnt_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t tmnt_vidmap (uint8_t** screen_start);

#endif /* TMNTSYSCALL_H */

