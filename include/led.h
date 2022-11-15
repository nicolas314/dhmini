#ifndef _LED_H_
#define _LED_H_

/* Work in progress */

int led_init(char * chipname, int pins[]);
int led_set(int r, int g, int b);
int led_blink(useconds_t usec);
void led_close(void);

#endif
