/* rtc.c - Functions to interact with the RTC
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc_drivers.h"
#include "process.h"
#include "scheduling.h"



int rtc_interrupt_occurred = 0;
int rtc_interrupt_counter = 0;

/* http://wiki.osdev.org/RTC#Setting_the_Registers */
/*
 * rtc_init
 *      SUMMARY: Initializes the RTC
 *       INPUTS: none
 *      OUTPUTS: none
 *       RETURN: none
 * SIDE EFFECTS: Writes to RTC register B, enables RTC IRQ
 */
void rtc_init(void)
{
    /* Local variables */
    uint32_t flags; /* Variable for storing the flags */
    
    cli_and_save(flags);
    
    /* Write the init keyword to RTC register B */
    outb(RTC_REGISTER_B, RTC_PORT);
    outb(RTC_INIT, RTC_PORT + 1);
    
    enable_irq(RTC_IRQ);
    
    restore_flags(flags);
}

/* http://wiki.osdev.org/RTC#Interrupts_and_Register_C */
/*
 * rtc_interrupt
 *      SUMMARY: Function called by the RTC interrupt handler
 *       INPUTS: none
 *      OUTPUTS: none
 *       RETURN: none
 * SIDE EFFECTS: Flushes register C
 */
void rtc_interrupt(void)
{
    /* Local variables */
    uint32_t flags; /* Variable for storing the flags */
    
    cli_and_save(flags);
    
    /* Flushing RTC register C */
    outb(RTC_REGISTER_C, RTC_PORT);
    inb(RTC_PORT + 1);
    int i = 0;
    int mod_val = 0;

   for(i = 0; i < NUM_TASKS; i++)
    {
        
        // Handle the unititialied case since rtc starts before scheduler info gets initialized
        if (pcb_all_started >= (NUM_TASKS - 1)){
            virtual_frequency = (int)(tasks[i].pcb->rtc_freq);
            if (virtual_frequency == 0){
                virtual_frequency = INITIAL_FREQUENCY;
            }
        }
        else {
            virtual_frequency = INITIAL_FREQUENCY;
        }
        // Calculate the value that will yield the correct psuedo-frequency
        mod_val = MAX_HZ / virtual_frequency;
        if (rtc_virtual_interrupt_counter % mod_val == 0){
            tasks[i].pcb->rtc_interrupt_occurred = 1;
        }  
    } 
    
    restore_flags(flags);
}
