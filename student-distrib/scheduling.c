/* scheduling.c */


#include "scheduling.h"
#include "pit_drivers.h"
#include "process.h"
#include "types.h"
#include "lib.h"
#include "sys_call.h"
#include "terminal.h"
#include "rtc_drivers.h"
#include "x86_desc.h"
#include "paging.h"
#include "i8259.h"
#include "modex.h"
#include "jank_malloc.h"

// Global vars for use with handlers, shell startup, virtualization, etc.
 int pit_interrupt_counter = 0;
 int pcb_all_started = 0;
 int scheduler_started = 0;

/* start_scheduler
 * Description: Allows scheduler to begin by initializing the task structs
 * and initializiating the PIT and executing the first shell.
 * Inputs: None
 * Outputs: None
 */
void start_scheduler(void)
{
    /* Local variables */
    uint32_t i; /* Iteration variable */
    
    /* Clear interrupts */
    cli();
    
    /* Initializing the task structures */
    for (i = 0; i < NUM_TASKS; i++)
    {
        tasks[i].started = !i; /* Evaluates to false for all values but 0 */
        tasks[i].terminal = &(terminals[i]);
    }
    
    /* Mark that the starting task is 0 */
    current_task = 0;

    /* Initialize the PIT */
    init_pit();

    set_mode_X();

    play_turtles();
    
    clear_mode_X();



    terminal_open(0);

    // test_malloc();
    // test_free();
    
    /* Indicate that the scheduler has started */
    scheduling_started = TRUE;
 
     /* Execute the first shell */
    sys_execute((uint8_t*)"shell");
    sti();
}

/*
 * schedule
 *      SUMMARY: Function called by the PIT interrupt handler. Switches between tasks
 *       every ~25 ms.
 *       INPUTS: none
 *      OUTPUTS: none
 * SIDE EFFECTS: Kicks off scheduling algorithm 
 */
void schedule(void)
{
    /* Local variables */
    task_t* curr; /* Pointer to the current and next task structures */
    task_t* next;

    /* Getting the current task */
    curr = &(CURRENT_TASK);
    
    /* Start critical section */
    cli_and_save(curr->flags);
    
    /* Getting the values of esp and ebp for scheduling */
    asm volatile ("        \n\
            movl %%esp, %0 \n\
            movl %%ebp, %1 \n\
            "
            : "=r"(curr->pcb->schedule_esp), "=r"(curr->pcb->schedule_ebp)
            : 
    );
    
    /* Storing the value of ESP0 */
    curr->pcb->schedule_esp0 = tss.esp0;

    /* Advance to and get the next task */
    current_task = (current_task + 1) % NUM_TASKS;
    next = &(CURRENT_TASK);

    /* Remap kernel and user video memory if the next tasks terminal is the active terminal */
    if(next->terminal->active)
    {
        map_virt_to_phys((uint8_t*)VIDMEM,(uint8_t*)VIDMEM);
        map_virt_to_phys((uint8_t*)VIRTUAL_END,(uint8_t*)VIDMEM);
    }
    else
    {
        map_virt_to_phys((uint8_t*)VIDMEM,(uint8_t*)next->terminal->video_mem);
        map_virt_to_phys((uint8_t*)VIRTUAL_END,(uint8_t*)next->terminal->video_mem);
    }
    
    /* Update the video params to next terminal's */
    get_video_params(&(curr->terminal->screen_x), &(curr->terminal->screen_y));
    set_video_params(next->terminal->screen_x, next->terminal->screen_y);
    
    //need to update 4mB user memory page every time process switch occurs
    uint32_t new_phys_addr = (PID_OFFSET + next->pid) * FOUR_M;
    page_modify(VIRTUAL_BEGIN, new_phys_addr, USER_PRIV);
    flush_TLB();

    /* If the next task has not yet been started, then we must start a shell on it */
    if (!next->started)
    {
        next->started = TRUE;
        send_eoi(PIT_IRQ);
        sys_execute((uint8_t*)"shell");
    }
    
    /* Restore important/"global" data */
    global_pid = next->pid;
    tss.esp0 = next->pcb->schedule_esp0;
    
    /* Update the cursor on the active terminal */
    update_cursor_active();
    
    /* End critical section */
    restore_flags(next->flags);
    
    /* Context switch using the next tasks ESP and EBP to switch into other task */
    asm volatile ("        \n\
            movl %0, %%esp \n\
            movl %1, %%ebp \n\
            leave          \n\
            ret            \n\
            "
            : 
            : "r"(next->pcb->schedule_esp), "r"(next->pcb->schedule_ebp)
            : "esp", "ebp"
    );
    
}


