#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gpiod.h>

struct {
    int    pins[3] ;
    struct gpiod_chip * chip ;
    struct gpiod_line * line[3] ;
} dh_led ;

int led_init(char * chipname, int pins[])
{
    int i ;
    memset(&dh_led, 0, sizeof(dh_led));
    dh_led.pins[0] = pins[0];
    dh_led.pins[1] = pins[1];
    dh_led.pins[2] = pins[2];

    dh_led.chip = gpiod_chip_open_by_name(chipname);
    dh_led.line[0] = gpiod_chip_get_line(dh_led.chip, pins[0]);
    dh_led.line[1] = gpiod_chip_get_line(dh_led.chip, pins[1]);
    dh_led.line[2] = gpiod_chip_get_line(dh_led.chip, pins[2]);
}

int led_set(int r, int g, int b)
{
    gpiod_line_request_output(dh_led.line[0], "gpio_led_r", r);
    gpiod_line_request_output(dh_led.line[1], "gpio_led_g", g);
    gpiod_line_request_output(dh_led.line[2], "gpio_led_b", b);
}

int led_blink(useconds_t usec)
{
    while (1) {
        led_set(0,0,0);
        usleep(usec);
        led_set(1,1,1);
        usleep(usec);
    }
}

void led_close(void)
{
    if (dh_led.line[0]) gpiod_line_release(dh_led.line[0]);
    if (dh_led.line[1]) gpiod_line_release(dh_led.line[1]);
    if (dh_led.line[2]) gpiod_line_release(dh_led.line[2]);
    if (dh_led.chip)    gpiod_chip_close(dh_led.chip);
    return ;
}

#ifdef TEST_MAIN
int main(int argc, char * argv[])
{
    int pins[3]={17, 27, 22};
    int r, g, b ;
	if (argc!=4) {
		printf("use: %s R G B\n", argv[0]);
		return 1 ;
	}
    r = (int)atoi(argv[1]);
    g = (int)atoi(argv[2]);
    b = (int)atoi(argv[3]);
    /*
    if (argc<2) {
        printf("use %s usec\n", argv[0]);
        return -1 ;
    }
    */

    led_init("gpiochip0", pins);
    led_set(r, g, b);
    /* led_blink((useconds_t)atoi(argv[1])); */
    led_close();
	return 0 ;
}
#endif
