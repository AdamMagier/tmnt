/* mouse.h */

#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"

#define MOUSE_PORT   0x60
#define MOUSE_CMD    0x64
#define MOUSE_STATUS 0x64

#define MOUSE_ENABLE_STREAMING 0xF4
#define MOUSE_ENABLE_AUXILIARY 0xA8
#define MOUSE_ENABLE_IRQ12 0x02
#define MOUSE_DISABLE_CLOCK 0x20
#define MOUSE_SET_DEFAULTS 0xF6
#define MOUSE_SET_SAMPLE_RATE 0xF3
#define MOUSE_CMD_BYTE 0xD4
#define MOUSE_SET_COMPAQ 0x60
#define MOUSE_GET_COMPAQ 0x20
#define MOUSE_GET_MOUSEID 0xF2
#define MOUSE_RATE_200 0xC8
#define MOUSE_RATE_100 0x64
#define MOUSE_RATE_80  0x50

#define MOUSE_IRQ 0x0C

#define MOUSE_BTN_M 0x04
#define MOUSE_BTN_R 0x02
#define MOUSE_BTN_L 0x01

#define MOUSE_SIGN_X 0x10
#define MOUSE_SIGN_Y 0x20

#define POSITIVE 0x00
#define NEGATIVE 0x01

#define LOWEST_BIT_MASK 0x01

#define UP   0x00
#define DOWN 0x01

#define TOP_TWO_BIT_MASK 0xC0

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t sign_x : 1;
    uint8_t sign_y : 1;
    uint8_t btn_l : 1;
    uint8_t btn_m : 1;
    uint8_t btn_r : 1;
} mouse_input_t;

mouse_input_t mouse_input;

void mouse_init(void);

void mouse_interrupt(void);

void mouse_get_input(void);

#endif /* _MOUSE_H */
