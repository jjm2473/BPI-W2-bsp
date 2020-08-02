#ifndef _TIMER_H
#define _TIMER_H

int timer_init(void);
unsigned long get_timer(unsigned long base);
void udelay(unsigned long usec);
void mdelay(unsigned long msec);

#endif