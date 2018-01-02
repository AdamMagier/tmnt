/* keyboard.h - Defines used in interactions with the keyboard
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

/* Bitmask to expose only the MSB */
#define MSB_MASK 0x80

/* Max size of the input buffer (128) */
#define INPUT_BUFFER_SIZE 0x80

/* Command code returned whenever CTRL + key is pressed */
#define CTRL_L 0xB3
#define CTRL_C 0xBC

/* Command codes returned whenever ALT + F# is pressed */
#define ALT_F1  0xF1
#define ALT_F2  0xF2
#define ALT_F3  0xF3
#define ALT_F4  0xF4
#define ALT_F5  0xF5
#define ALT_F6  0xF6
#define ALT_F7  0xF7
#define ALT_F8  0xF8
#define ALT_F9  0xF9
#define ALT_F10 0xFA
#define ALT_F11 0xFB
#define ALT_F12 0xFC

/* ASCII codes for special characters */
#define NEWLINE 0x0A
#define NONE    0x00
#define BKSP    0x08
#define TAB     0x09

/* Flags for determining if input is in progress */
#define INPUT_ENDED 0x00
#define IN_PROGRESS 0xFF

/* Number of allowed terminals (should be the same as NUM_TASKS) */
#define NUM_TERMINALS 0x03

/* Address of the video memory in physical memory */
#define VIDEO_MEM 0xB8000

/* Addresses to relevant video memories/buffer */
#define BASE_VIDEO_MEM       0xB8000
#define TERMINAL_1_VIDEO_MEM 0xB9000
#define TERMINAL_2_VIDEO_MEM 0xBA000
#define TERMINAL_3_VIDEO_MEM 0xBB000

/* "Boolean" flags for determining if a given terminal is active */
#define ACTIVE   0xFF
#define INACTIVE 0x00

/* Prints out the character passed in (used for debugging) */
#define TERMINAL_DEBUG(char) do { putc(char); } while (0)
    
/* Prints out a warning message regarding the stability of this operating system */
#define WARNING printf("WARNING: THE OPERATING SYSTEM IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n\nIN NO EVENT SHALL THE TEENAGE MUTEX NINJA TURTLES OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\nTHE TEENAGE MUTEX NINJA TURTLES ARE NOT LIABLE FOR ANY INSTABILITY OR UNWARRENTED OUTCOME THAT IS THE RESULT OF CHANGING THIS OPERATING SYSTEM.\n\nBY CONTINUING YOU ARE AGREEING TO THESE TERMS AND CONDITIONS\n-----\n")

/* Macro to abstract the indexing into the active terminal */
#define ACTIVE_TERMINAL terminals[active_terminal]

/* Struct for holding the terminal for task switching */
typedef struct {
    uint32_t terminal_number; /* Index into the terminal array */
    uint8_t* video_mem; /* Private video address for buffer */
    uint32_t screen_x; /* Holds the coords of the current display location */
    uint32_t screen_y;
    uint8_t input_buffer[INPUT_BUFFER_SIZE]; /* Input buffer from command line */
    uint8_t buffer_index; /* Index into the input buffer */
    volatile uint8_t input_status; /* Status of the input (i.e. has enter been pressed) */
    uint8_t active; /* Flag for indicating whether or not this terminal is active */
} terminal_t;

/* Array of terminals that can be switched between */
terminal_t terminals[NUM_TERMINALS];

/* Index into the terminal array */
volatile uint32_t active_terminal;

/* Open the terminal */
extern int32_t terminal_open(const uint8_t* filename);

/* Read from the terminal */
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* Write to the terminal */
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* Close the terminal */
extern int32_t terminal_close(int32_t fd);

/* Called on keyboard interrupt */
extern void terminal_interrupt(void);

#endif /* _TERMINAL_H */
