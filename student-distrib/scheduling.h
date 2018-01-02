/* scheduling.h */

#ifndef _SCHEDULING_H

#define _SCHEDULING_H

#include "types.h"
#include "process.h"
#include "terminal.h"

/* Number of tasks that can run in the scheduler (should be the same as NUM_TERMINALS) */
#define NUM_TASKS 0x03

/* Macro to abstract indexing into the task array */
#define CURRENT_TASK tasks[current_task]

/* Task struct for use with scheduler */
typedef struct {
    pcb_t* pcb; /* Pointer to the PCB of the process currently executing in the task */
    uint32_t started; /* Flag for determining if a terminal/shell has been started in the task */
    uint32_t flags; /* Save value of the flags */
    uint32_t pid; /* PID of the process that is currently executing in the task */
    terminal_t* terminal; /* Pointer to the terminal struct of the current task */
} task_t;

/* Array of the task structures */
task_t tasks[NUM_TASKS];

/* Index into the task array */
uint32_t current_task;

// Global vars for use with handlers, shell startup, virtualization, etc.
extern int pcb_all_started;

/* Flag for determining if scheduling has started or not */
volatile uint32_t scheduling_started;

/* Counter for tracking number of interrupts from the PIT */
extern int pit_interrupt_counter;

/* Start the PIT and execute the first shell */
extern void start_scheduler(void);

/* Function for PIT interrupts */
extern void schedule(void);

#endif
