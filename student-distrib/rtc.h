/* rtc.h - Defines used in interactions with the RTC
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* Port number for the RTC */
#define RTC_PORT 0x70

/* Values to access the specific RTC registers */
#define RTC_REGISTER_A 0x8A
#define RTC_REGISTER_B 0x8B
#define RTC_REGISTER_C 0x8C

/* Initialization keyword for the RTC */
#define RTC_INIT 0x44

/* IRQ number for the RTC */
#define RTC_IRQ 0x08

/* Initialize the RTC */
void rtc_init(void);
/* Function for RTC interrupts */
void rtc_interrupt(void);

extern int rtc_interrupt_counter;
extern int rtc_interrupt_occurred;

#endif /* _RTC_H */
