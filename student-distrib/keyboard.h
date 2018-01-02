/* keyboard.h - Defines used in interactions with the keyboard
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

/* Keyboard ports */
#define KEYBOARD_PORT   0x60
#define KEYBOARD_CMD    0x64
#define KEYBOARD_STATUS 0x64

#define KEYBOARD_IRQ    0x01

/* Initialize the keyboard */
void keyboard_init(void);
/* Translate and print a key press */
uint8_t keyboard_interrupt(void);

#endif /* _KEYBOARD_H */
