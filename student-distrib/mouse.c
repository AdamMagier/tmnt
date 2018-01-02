/* mouse.c */

/*** Courtesy of SANiK of OSDev Forums ***/

#include "mouse.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"

uint8_t mouse_cycle = 0xFF;
uint8_t mouse_packets[4];
uint8_t mouse_x;
uint8_t mouse_y;
uint8_t mouse_z;
uint8_t mouse_sign_x;
uint8_t mouse_sign_y;
uint8_t mouse_btn_l;
uint8_t mouse_btn_m;
uint8_t mouse_btn_r;
volatile uint8_t mouse_input_status = 0xFF;


//Mouse functions
void mouse_interrupt(void)
{
    switch(mouse_cycle)
    {
        case 0xFF: // The first interrupt fucks things up a bit
            mouse_cycle++;
            break;
        case 0x00:
            mouse_packets[0] = inb(MOUSE_PORT);
            mouse_cycle++;
            break;
        case 0x01:
            mouse_packets[1] = inb(MOUSE_PORT);
            mouse_cycle++;
            break;
        case 0x02:
            mouse_packets[2] = inb(MOUSE_PORT);
            mouse_cycle++;
            break;
        case 0x03:
            mouse_packets[3] = inb(MOUSE_PORT);
            if (!(mouse_packets[0] & TOP_TWO_BIT_MASK)) // Detecting garbage packets
            {
                mouse_sign_x = mouse_packets[0] & MOUSE_SIGN_X;
                mouse_sign_y = mouse_packets[0] & MOUSE_SIGN_Y;
                mouse_btn_l = mouse_packets[0] & MOUSE_BTN_L;
                mouse_btn_m = mouse_packets[0] & MOUSE_BTN_M;
                mouse_btn_r = mouse_packets[0] & MOUSE_BTN_R;
                mouse_x = mouse_packets[1];
                mouse_y = mouse_packets[2];
                mouse_z = mouse_packets[3];
                mouse_input_status = 0x00;
                update_cursor_mouse();
            }
            mouse_cycle = 0x00;
            break;
    }
}

void mouse_get_input(void)
{
    while (mouse_input_status);
    
    mouse_input.x = mouse_x;
    mouse_input.y = mouse_y;
    mouse_input.z = mouse_z;
    mouse_input.sign_x = (mouse_sign_x ? NEGATIVE : POSITIVE);
    mouse_input.sign_y = (mouse_sign_y ? NEGATIVE : POSITIVE);
    mouse_input.btn_l = (mouse_btn_l ? DOWN : UP);
    mouse_input.btn_m = (mouse_btn_m ? DOWN : UP);
    mouse_input.btn_r = (mouse_btn_r ? DOWN : UP);
    
    mouse_input_status = 0xFF;
}

// 0 -> wait for read, 1 -> wait for write
inline void mouse_wait(uint8_t a_type)
{
    uint32_t _time_out=100000; //unsigned int
    if(a_type==0)
    {
        while(_time_out--) //Wait for data to be available for reading
        {
            if((inb(MOUSE_CMD) & 1)==1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while(_time_out--) //Wait for commands to be available to write
        {
            if((inb(MOUSE_CMD) & 2)==0)
            {
                return;
            }
        }
        return;
    }
}

inline void mouse_write(uint8_t a_write)
{
    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    outb(MOUSE_CMD_BYTE, MOUSE_CMD);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    outb(a_write, MOUSE_PORT);
}

uint8_t mouse_read()
{
    //Get's response from mouse
    mouse_wait(0); 
    return inb(MOUSE_PORT);
}

void mouse_init(void)
{
    uint8_t _status;    //unsigned char

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outb(MOUSE_ENABLE_AUXILIARY, MOUSE_CMD);
    
    //Enable the interrupts
    mouse_wait(1);
    outb(MOUSE_GET_COMPAQ, MOUSE_CMD);
    mouse_wait(0);
    _status=(inb(MOUSE_PORT) | MOUSE_ENABLE_IRQ12) & ~MOUSE_DISABLE_CLOCK;
    mouse_wait(1);
    outb(MOUSE_SET_COMPAQ, MOUSE_CMD);
    mouse_wait(1);
    outb(_status, MOUSE_PORT);
    
    //Tell the mouse to use default settings
    mouse_write(MOUSE_SET_DEFAULTS);
    mouse_read();    //Acknowledge
    
    //Enable the mouse
    mouse_write(MOUSE_ENABLE_STREAMING);
    mouse_read();    //Acknowledge
    
    //Enable scroll wheel using magic sequence
    mouse_write(MOUSE_SET_SAMPLE_RATE); // Step 1: set rate to 200
    mouse_read();
    mouse_write(MOUSE_RATE_200);
    mouse_read();    //Acknowledge
    
    mouse_write(MOUSE_SET_SAMPLE_RATE); // Step 2: set rate to 100
    mouse_read();
    mouse_write(MOUSE_RATE_100);
    mouse_read();    //Acknowledge
    
    mouse_write(MOUSE_SET_SAMPLE_RATE); // Step 3: set rate to 80
    mouse_read();
    mouse_write(MOUSE_RATE_80);
    mouse_read();    //Acknowledge
    
    //Set mouse to function at higher sample rate
    mouse_write(MOUSE_SET_SAMPLE_RATE);
    mouse_read();
    mouse_write(MOUSE_RATE_200);
    mouse_read();    //Acknowledge
    
    mouse_write(MOUSE_GET_MOUSEID);
    mouse_read();    //Acknowledge
    printf("Current MouseID: %x\n", mouse_read());

    //Setup the mouse handler
    enable_irq(MOUSE_IRQ);
}
