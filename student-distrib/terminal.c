/* keyboard.c - Functions to interact with the keyboard
 * vim:ts=4 noexpandtab
 */

#include "terminal.h"
#include "keyboard.h"
#include "scancode_ascii_lookup.h"
#include "lib.h"
#include "sys_call.h"
#include "i8259.h"
#include "scheduling.h"
#include "process.h"
#include "paging.h"
/* File specific variables */

/* The input buffer (duh) */
static uint8_t input_buffer[INPUT_BUFFER_SIZE];
/* Current index into the buffer */
static uint8_t buffer_index;

/* Flag for determining whether or not there is current input being collected */
static volatile uint8_t input_status;

/* File specific functions - see headers */
static void alt_f1(void);
static void alt_f2(void);
static void alt_f3(void);
static void ctrl_c(void);
static void ctrl_l(void);
static void reset_buffer(void);
static void switch_terminals(uint32_t next_terminal);


/*
 * terminal_open
 *   DESCRIPTION: Opens the terminal
 *   INPUTS: Unused filename for consistency with system call params
 *   RETURN VALUE: Always 0
 *   SIDE EFFECTS: Initialize the keyboard; reset the screen and clear the buffer
 */
int32_t terminal_open(const uint8_t* filename)
{
    /* Local variables */
    uint32_t i; /* Iteration variable */
    uint32_t flags;
    
    /* Start critical section */
    cli_and_save(flags);
    
    /* Initialize the keyboard */
    keyboard_init();
    
    /* Reset the screen and buffer */
    reset_screen();
    reset_buffer();
    
    /* Signal that there has been no input started yet */
    input_status = INPUT_ENDED;
    
    /* Initialize the terminals */
    for (i = 0; i < NUM_TERMINALS; i++)
    {
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].buffer_index = 0;
        terminals[i].input_status = INPUT_ENDED;
        terminals[i].terminal_number = i;
        terminals[i].active = !i;
        terminals[i].video_mem = (uint8_t*)(BASE_VIDEO_MEM + (i + 1) * FOUR_K);
        map_virt_to_phys(terminals[i].video_mem, terminals[i].video_mem);
        memset(terminals[i].input_buffer, NEWLINE, INPUT_BUFFER_SIZE);
    }
    memset(input_buffer, NEWLINE, INPUT_BUFFER_SIZE);
    
    /* Set the active terminal to be the first */
    active_terminal = 0;
    map_virt_to_phys((uint8_t*) BASE_VIDEO_MEM, (uint8_t*) BASE_VIDEO_MEM);
    map_virt_to_phys(ACTIVE_TERMINAL.video_mem, (uint8_t*) BASE_VIDEO_MEM);
    
    /* Reset screen coords */
    set_video_params(0, 0);
    
    restore_flags(flags);
    
    /* Return success */
    return 0;
}


/*
 * terminal_read
 *   DESCRIPTION: reads FROM the keyboard buffer into buf
 *   INPUTS: buf: A buffer to copy keyboard input into
 *              nbytes: The number of bytes (chars) to read
 *   RETURN VALUE: Number of bytes read
 *   SIDE EFFECTS: Reads nbytes of keyboard input into the param buf
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    /* Local variables */
    int32_t num_copied; /* Number of bytes copied */
    uint8_t* char_buf;  /* Casted version of buffer arg */
    terminal_t* current_terminal;
    
    /* Checking valid parameters */
    if (!buf)
    {
        return -1;
    }
    
    if (nbytes < 0)
    {
        return -1;
    }
    
    /* Casting buffer */
    char_buf = (uint8_t*) buf;
    
    current_terminal = CURRENT_PCB_ADDRESS->terminal;
    
    /* Signal that input is occurring atm */
    current_terminal->input_status = IN_PROGRESS;
    
    /* Spin until input is over */
    while (current_terminal->input_status);
    
    /* Determine how many bytes to copy based on index and requested number */
    if (current_terminal->buffer_index < nbytes)
    {
        num_copied = current_terminal->buffer_index;
    }
    else
    {
        num_copied = nbytes;
    }
    
    /* Copy input buffer into the arg buffer and reset input */
    memcpy(char_buf, current_terminal->input_buffer, num_copied);
    reset_buffer();
    
    /* Return the number of bytes copied */
    return num_copied;
}


/*
 * terminal_write
 *   DESCRIPTION:  Writes TO the screen from buf.
 *   INPUTS: buf: A buffer of chars to write to the screen
 *             nbytes: The number of bytes (chars) to write
 *   OUTPUTS: Screen display
 *   RETURN VALUE: Number of bytes written, or -1.
 *   SIDE EFFECTS: All data is displayed to the screen immediately.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    /* Local variables */
    int i;               /* Iteration variable */
    uint8_t* bufptr;     /* Casted version of arg buffer */
    unsigned long flags; /* Save variable for flags */
    
    /* Checking for invalid params */
    if (!buf)
    {
        return -1;
    }
    
    if (nbytes < 0)
    {
        return -1;
    }
    
    /* Start critical section */
    cli_and_save(flags);

    /* Casting arg buf */
    bufptr = (uint8_t*)buf;
    
    /* Printing what's inside the arg buf */
    for (i = 0; i < nbytes; i++)
    {
        putc(bufptr[i]);
    }
    
    /* End critical section */
    restore_flags(flags);
    
    /* Return the number written */
    return i;
}


/*
 * terminal_close
 *   DESCRIPTION: Close the terminal. Complete the close syscall for the terminal case.
 *   INPUTS: fd, unused; included for consistency with syscall
 *   OUTPUTS: None
 *   RETURN VALUE: Always 0 
 *   SIDE EFFECTS: None
 */
int32_t terminal_close(int32_t fd)
{
    return 0;
}


/*
 * terminal_interrupt
 *   DESCRIPTION: Interprets a key press and prints it to the screen.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Prints an alphanumeric or symbolic key press to the screen.
 */
void terminal_interrupt(void)
{
    /* Local variables */
    uint8_t scancode;   /* Scancode received from the keyboard */
    uint8_t ascii_data; /* ASCII representation of that scancode */

    /* Get the scancode on keyborad interrupt and map it to the ASCII character */
    scancode = keyboard_interrupt();
    ascii_data = translate_scancode(scancode);
    
    /* TERMINAL_DEBUG(ascii_data); */
    
    /* Analyzing the ASCII data */
    if (ascii_data & MSB_MASK)
    {
        /* Only commands come back as having MSB == 1 (design decision) */
        switch (ascii_data)
        {
            /* Run the corresponding command if the ASCII code matches */
            case ALT_F1:
                alt_f1();
                break;
            case ALT_F2:
                alt_f2();
                break;
            case ALT_F3:
                alt_f3();
                break;
            case CTRL_C:
                ctrl_c();
                break;
            case CTRL_L:
                ctrl_l();
                break;
            default:
                /* In theory should never occur... */
                break;
        }
    }
    else
    {
        /* It's a regular keypress so decide what to do */
        switch (ascii_data)
        {
            case NONE:
                /* ASCII code 0 does nothing (design decision) */
                break;
            case TAB:
                /* No functionality for tab...YET! */
                break;
            case BKSP:
                /* Remove previous character from screen and buffer */
                if (ACTIVE_TERMINAL.buffer_index > 0)
                {
                    putc_active(ascii_data);
                    ACTIVE_TERMINAL.buffer_index--;
                    ACTIVE_TERMINAL.input_buffer[ACTIVE_TERMINAL.buffer_index] = NEWLINE;
                }
                break;
            case NEWLINE:
                /* Print newline and signal that input is over */
                putc_active(ascii_data);
                ACTIVE_TERMINAL.input_status = INPUT_ENDED;
                ACTIVE_TERMINAL.buffer_index++;
                break;
            default:
                /* Print character and write to buffer only if there's space */
                if (ACTIVE_TERMINAL.buffer_index < INPUT_BUFFER_SIZE - 1)
                {
                    putc_active(ascii_data);
                    ACTIVE_TERMINAL.input_buffer[ACTIVE_TERMINAL.buffer_index] = ascii_data;
                    ACTIVE_TERMINAL.buffer_index++;
                }
                break;
        }
    }
}


/*
 * alt_f1
 *   DESCRIPTION: Runs the command sequence invoked by ALT + F1 key combo
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the screen and reprints the curren input
 */
void alt_f1(void)
{
    uint32_t flags;
    
    cli_and_save(flags);
    /* Local variables */
    
    if (active_terminal != 0)
    {
        switch_terminals(0);
    }
    
    restore_flags(flags);
    
}


/*
 * alt_f2
 *   DESCRIPTION: Runs the command sequence invoked by ALT + F2 key combo
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the screen and reprints the curren input
 */
void alt_f2(void)
{
    uint32_t flags;
    
    cli_and_save(flags);
    /* Local variables */
    
    if (active_terminal != 1)
    {
        switch_terminals(1);
    }
    
    restore_flags(flags);
}


/*
 * alt_f3
 *   DESCRIPTION: Runs the command sequence invoked by ALT + F3 key combo
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the screen and reprints the curren input
 */
void alt_f3(void)
{
    uint32_t flags;
    
    cli_and_save(flags);
    /* Local variables */
    
    if (active_terminal != 2)
    {
        switch_terminals(2);
    }
    
    restore_flags(flags);
    
}


/*
 * ctrl_c
 *   DESCRIPTION: Runs the command sequence invoked by CTRL + C key combo
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the screen and reprints the curren input
 */
void ctrl_c(void)
{
    uint32_t flags;
    terminal_t* current_terminal;
    
    cli_and_save(flags);
    /* Local variables */
    /* None at the moment */
    current_terminal = CURRENT_PCB_ADDRESS->terminal;
    /* Call sys_halt on the user side */
    current_terminal->input_status = INPUT_ENDED;
    reset_buffer();
    send_eoi(KEYBOARD_IRQ);
    sys_halt(0);
    
    restore_flags(flags);
}


/*
 * ctrl_l
 *   DESCRIPTION: Runs the command sequence invoked by CTRL + L key combo
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the screen and reprints the curren input
 */
void ctrl_l(void)
{
    uint32_t flags;
    uint8_t i; /* Iteration variable */
    
    cli_and_save(flags);
    /* Local variables */
    
    /* Reset screen and redraw current input */
    reset_screen_active();
    for (i = 0; i < buffer_index; i++)
    {
        putc_active(ACTIVE_TERMINAL.input_buffer[i]);
    }
    
    restore_flags(flags);
}


/*
 * reset_buffer
 *   DESCRIPTION: Fill the buffer with newline chars and reset the index.
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Clears the buffer and resets the buffer index
 */
void reset_buffer(void)
{
    uint32_t flags;
    
    cli_and_save(flags);
    /* Overwrite the buffer and reset the index */
    memset(ACTIVE_TERMINAL.input_buffer, NEWLINE, INPUT_BUFFER_SIZE);
    ACTIVE_TERMINAL.buffer_index = 0;
    
    restore_flags(flags);
}

/*
 * switch_terminals
 *   DESCRIPTION: Switch between terminals with Alt + F1/F2/F3 for CP5.
 *   INPUTS: 0, 1, or 2, value of the next terminal window
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Switches video display to the next terminal
 */
void switch_terminals(uint32_t next_terminal)
{
    /* Local variables */
    terminal_t* old_terminal; /* Pointers to the current terminal and */
    terminal_t* new_terminal; /* the terminal we are switching into */
    uint32_t flags; /* Save variable for the value of eflags */
    
    /* Start critical section */
    cli_and_save(flags);
    
    /* Updating terminal active statuses and indicating that there's a new active terminal */
    old_terminal = &(ACTIVE_TERMINAL);
    old_terminal->active = INACTIVE;
    active_terminal = next_terminal;
    new_terminal = &(ACTIVE_TERMINAL);
    new_terminal->active = ACTIVE;
    
    /* Unit mapping the relevant video addresses to make memcpys easier */
    map_virt_to_phys((uint8_t*)BASE_VIDEO_MEM,(uint8_t*)BASE_VIDEO_MEM);
    map_virt_to_phys(old_terminal->video_mem, old_terminal->video_mem);
    map_virt_to_phys(new_terminal->video_mem, new_terminal->video_mem);
    
    /* Copy the current video memory into the old terminal's buffer */
    memcpy(old_terminal->video_mem, (uint8_t*)BASE_VIDEO_MEM, FOUR_K);
    /* Copy the new terminal's buffer into the actual video memory */
    memcpy((uint8_t*)BASE_VIDEO_MEM, new_terminal->video_mem, FOUR_K);
    
    /* Mask the new terminals buffer address to the physical video memory */
    map_virt_to_phys(new_terminal->video_mem, (uint8_t*) BASE_VIDEO_MEM);
    
    if(CURRENT_TASK.terminal->active)
    {
        /* If the current task is now on the active terminal, map it's user video page to video memory */
        map_virt_to_phys((uint8_t*)VIRTUAL_END,(uint8_t*)BASE_VIDEO_MEM);
    }
    else
    {
        /* Otherwise just map to the new terminal's buffer */
        map_virt_to_phys((uint8_t*)BASE_VIDEO_MEM, new_terminal->video_mem);
        map_virt_to_phys((uint8_t*)VIRTUAL_END, new_terminal->video_mem);
    }
    
    /* Update the cursor on the active terminal */
    update_cursor_active();
    
    /* End critical section */
    restore_flags(flags);
}


