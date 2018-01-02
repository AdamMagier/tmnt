/* keyboard.c - Functions to interact with the keyboard
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"

/* keyboard_init
 * Inputs: None
 * Outputs: None
 * Side effects: Initializes the keyboard
 */
void keyboard_init(void)
{
    /* Local variables */
    uint32_t flags; /* Save variable for flags */
    
    cli_and_save(flags);
    
    /* Enable the keyboard's IRQ */
    enable_irq(KEYBOARD_IRQ);
    
    restore_flags(flags);
}

/* http://wiki.osdev.org/Keyboard#Enough.2C_give_me_code.21 */
/* keyboard_interrupt
 * Inputs: None
 * Outputs: None
 * Side effects: Interprets a key press into a scancode, and prints the scancode to the screen
 */
uint8_t keyboard_interrupt(void)
{
    return inb(KEYBOARD_PORT);
}
