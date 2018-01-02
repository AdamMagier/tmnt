/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC 
 * http://wiki.osdev.org/8259_PIC 
 * Inputs: None
 * Outputs: None
 */
void i8259_init(void)
{
    cli();
    
    /* Masking all interrupts */
    outb(MASK, MASTER_8259_PORT + 1);
    io_wait();
    outb(MASK, SLAVE_8259_PORT + 1);
    io_wait();
    
    /* Setting up master PIC */
    outb(ICW1, MASTER_8259_PORT);
    io_wait();
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    io_wait();
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    io_wait();
    outb(ICW4, MASTER_8259_PORT + 1);
    io_wait();
    
    /* Setting up slave PIC */
    outb(ICW1, SLAVE_8259_PORT);
    io_wait();
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    io_wait();
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    io_wait();
    outb(ICW4, SLAVE_8259_PORT + 1);
    io_wait();
    
    /* Masking all interrupts */
    outb(MASK, MASTER_8259_PORT + 1);
    io_wait();
    outb(MASK, SLAVE_8259_PORT + 1);
    io_wait();
    
    /* Delay a bit... */
    { volatile int ii; for (ii = 0; ii < 1000; ii++); }
    
    /* Need to enable the slave's IRQ */
    enable_irq(SLAVE_IRQ);
    
    sti();
}

/* enable_irq
 *  http://wiki.osdev.org/8259_PIC 
 * Inputs: an IRQ
 * Outputs: None
 * Enables (unmasks) the specified IRQ */
void enable_irq(uint32_t irq_num)
{
    uint8_t port;
    uint8_t value;
    uint32_t flags;
    
    cli_and_save(flags);
    
    if(irq_num < SLAVE_IRQ_START)
    {
        port = MASTER_8259_PORT + 1;
    }
    else
    {
        port = SLAVE_8259_PORT + 1;
        irq_num -= SLAVE_IRQ_START;
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
    
    restore_flags(flags);
}

/* disable_irq
 * http://wiki.osdev.org/8259_PIC 
 * Inputs: an IRQ
 * Outputs: None
 * Disables (masks) the specified IRQ */
void disable_irq(uint32_t irq_num)
{
    uint8_t port;
    uint8_t value;
    uint32_t flags;
    
    cli_and_save(flags);
    
    if(irq_num < SLAVE_IRQ_START)
    {
        port = MASTER_8259_PORT + 1;
    }
    else
    {
        port = SLAVE_8259_PORT + 1;
        irq_num -= SLAVE_IRQ_START;
    }
    value = inb(port) | (1 << irq_num);
    outb(value, port);
    
    restore_flags(flags);
}

/* Send end-of-interrupt signal for the specified IRQ */
/* http://wiki.osdev.org/8259_PIC */
void send_eoi(uint32_t irq_num)
{
    uint8_t eoi;
    uint32_t flags;
    
    cli_and_save(flags);
    
    if(irq_num >= SLAVE_IRQ_START)
    {
        eoi = EOI | (irq_num & SLAVE_IRQ_MASK);
        outb(eoi, SLAVE_8259_PORT);
        eoi = EOI | SLAVE_IRQ;
        outb(eoi, MASTER_8259_PORT);
    }
    else
    {
        eoi = EOI | irq_num;
        outb(eoi, MASTER_8259_PORT);
    }
    
    restore_flags(flags);
}
