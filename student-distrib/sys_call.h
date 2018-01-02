/* sys_call.h */

#ifndef _SYS_CALL_H

#define _SYS_CALL_H

#include "lib.h"

/* Header value for determining if a file is executable */
#define ELF_HEADER 0x464C457F

/* Bitmask for enabling interrupts in eflags */
#define ENABLE_INTERRUPTS 0x0200

/* Max size of the command from the command line (128) */
#define MAX_CMD_SIZE 0x80

/* ASCII value of the space " " character */
#define SPACE_CHAR 0x20

/* ASCII value of the newline "\n" character */
#define NEWLINE_CHAR 0x0A

/* The start of the virtual address space for user programs (128MB) */
#define PROGRAM_PAGE_START 0x08000000

/* Virtual address for where the executable program should be loaded */
#define PROGRAM_START_ADDRESS 0x08048000

/* Index into the file for where the entry point is located */
#define ENTRY_POINT_LOCATION 24

/* Starting address of the virtual user stack */
#define VIRTUAL_STACK_START 0x83FFFFC

/* Identifiers for determining file type */
#define FILETYPE_RTC 0
#define FILETYPE_DIRECTORY 1
#define FILETYPE_FILE 2

/* Terminate a process */
extern int32_t sys_halt(uint8_t status);

/* Attempts to load an execute a program */
extern int32_t sys_execute(const uint8_t* command);

/* Links the read syscall with the fd array fops pointer of the current process */
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

/*  Links the write syscall with the fd array fops pointer of the current process */ 
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

/* Provide access to the filesystem */
extern int32_t sys_open(const uint8_t* filename);

/* Closes the specified file descriptor and makes it available for return from later calls to open.*/
extern int32_t sys_close(int32_t fd);

/* Copies the command line args into the buffer */
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

/* Maps a page to video memory and overwrites the passed in value with that address */
extern int32_t sys_vidmap(uint8_t** screen_start);

/* Not to be implemented */
extern int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void);

/* "Dummy" function for building up an IRET stack for context switching */
extern void context_switch(uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss);

#endif /* _SYS_CALL_H */
