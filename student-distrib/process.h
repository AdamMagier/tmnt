/* process.h */

#ifndef _PROCESS_H
#define _PROCESS_H

#include "terminal.h"

/* Constants relating to commmon memory block sizes */
#define ONE_K 0x400
#define ONE_M (ONE_K * ONE_K)
#define FOUR_K (0x04 * ONE_K)
#define FOUR_M (0x04 * ONE_M)
#define EIGHT_K (0x08 * ONE_K)
#define EIGHT_M (0x08 * ONE_M)

/* Constants for commonly used memory addresses */
#define VIDMEM 0xB8000
#define VIRTUAL_BEGIN (ONE_M * 128)
#define VIRTUAL_END (ONE_M * 132)

/* Bitmask to wipe off the lower 13 bits to isolate the top of the kernel stack */
#define KERNEL_STACK_MASK 0xFFFFE000

/* Size of the file descriptor array */
#define FD_ARRAY_SIZE 0x08

/* Flag for file descriptors to show that the file is in use */
#define FD_IN_USE 0x80000000

/* Indexes into the file_ops arrays for the different operations */
#define FOPS_OPEN  0x00
#define FOPS_READ  0x01
#define FOPS_WRITE 0x02
#define FOPS_CLOSE 0x03

/* The first file descriptor is at 2 because stdin/out occupy slots 0 and 1 */
#define FIRST_FD 2

/* Constant used for calculating page offsets */
#define PID_OFFSET 2

/* Macro for calculating the bottom of the kernel stack for a given process number */
#define KERNEL_STACK_ADDRESS(process_number) (EIGHT_M - process_number * EIGHT_K - 0x04)

/* Macro which calculates the pointer of the PCB for a given process number */
#define PCB_ADDRESS(number) ((pcb_t*) (KERNEL_STACK_ADDRESS(number) & KERNEL_STACK_MASK))

/* Macro which returns the pointer to the current PCB */
#define CURRENT_PCB_ADDRESS (PCB_ADDRESS(global_pid))

/* The total number of processes that can be run */
#define NUM_PROCESSES 0x06

/* Flags for determining whether or not PID's are available */
#define AVAILABLE   0x00000000
#define UNAVAILABLE 0xFFFFFFFF

/* Maximum size of the args that can be gotten from the command line (128) */
#define MAX_ARG_SIZE 0x80

/* User priviledge level value */
#define USER_PRIV 3

/* Macro to get the first available PID */
#define FIRST_AVAILABLE_PID(number) do {            \
        number = 0;                                 \
        while(pid_availability[number])             \
        {                                           \
            number++;                               \
            if (number > NUM_PROCESSES)             \
            {                                       \
                number = UNAVAILABLE;               \
                break;                              \
            }                                       \
        }                                           \
        if (number != UNAVAILABLE)                  \
        {                                           \
            pid_availability[number] = UNAVAILABLE; \
        }                                           \
    } while (0)

/* Macro for marking the passed in PID as being available */
#define REMOVE_PID(number) do { pid_availability[number] = AVAILABLE; } while (0)

/* Generic function pointers for use in the file_ops in the file descriptor */
typedef int32_t (*open_t)(const uint8_t * filename);
typedef int32_t (*read_t)(int32_t fd, void* buf, int32_t nbytes);
typedef int32_t (*write_t)(int32_t fd, const void* buf, int32_t nbytes);
typedef int32_t (*close_t)(int32_t fd);

/* A struct used for the file descriptor array */
typedef struct fd_entry {
    int32_t* file_ops;
    uint32_t inode;
    uint32_t position;
    uint32_t flags;
} fd_entry_t;
typedef struct pcb pcb_t;
/* PCB struct */
 struct pcb {
    fd_entry_t fd_array[FD_ARRAY_SIZE];
    
    /* Pointer to the PCB of the parent process */
    pcb_t* parent_pcb;
    
    /* PID of the parent process */
    uint32_t parent_pid;
    
    /* ESP of the parent's call to sys_execute; used to exit from sys_halt */
    uint32_t parent_esp;
    
    /* EBP of the parent's call to sys_execute; used to exit from sys_halt */
    uint32_t parent_ebp;
    
    /* ESP0 of the parent process; used while exiting from sys_halt */
    uint32_t parent_esp0;
    
    /* ESP of the current process for when the scheduler interrupts; used in the schedule function */
    uint32_t schedule_esp;
    
    /* EBP of the current process for when the scheduler interrupts; used in the schedule function */
    uint32_t schedule_ebp;
    
    /* ESP0 of the current process for when the scheduler interrupts; used in the schedule function */
    uint32_t schedule_esp0;
    
    /* The number of the terminal that the process is currently executing on */
    uint32_t terminal_number;
    
    /* Frequency that the RTC is programmed to; used in virtualization of the RTC */
    uint32_t rtc_freq;
    
    /* Flag for signaling that an RTC interrupt has occured */
    uint32_t rtc_interrupt_occurred;
    
    /* Pointer to the terminal struct that the current process is executing on */
    terminal_t* terminal;
    
    /* Array for holding the command line args; used in sys_getargs */
    uint8_t args[MAX_ARG_SIZE];
};

/* PID of the process that is currently running; used in calculating PCB address */
uint32_t global_pid;

/* Array containing flags for whether or not a given PID is available */
uint32_t pid_availability[NUM_PROCESSES];

#endif /* _PROCESS_H */
