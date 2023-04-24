#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "dhmini.h"

#define PACK_RGB(r,g,b) (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3)

void randomize(uint16_t * rect, uint16_t * color)
{
    uint8_t r, g, b ;

    rect[2] = rand() % rect[0] ;
    rect[3] = rand() % rect[1] ;
    rect[4] = rand() % rect[0] ;
    rect[5] = rand() % rect[1] ;

    r = 128 + rand() % 128 ;
    g = 128 + rand() % 128 ;
    b = 128 + rand() % 128 ;

    *color = PACK_RGB(r,g,b);
    return ;
}

#define DELAY 1

int main(void) {
    int i ;
    uint16_t r[6];
    uint16_t color ;
    dh_config_t config = {
        .spi_path = "/dev/spidev0.1",
        .spi_speed = 80000000,
        .gpio_chip_name = "gpiochip0",
        .dc_pin = 9,
        .bl_pin = 13,
        .width = DHMINI_WIDTH,
        .height = DHMINI_HEIGHT
    };
    uint16_t colors[] = {
        C_BLACK, C_DKGREY, C_LTGREY, C_WHITE,
        C_RED, C_BLUE, C_GREEN, C_BROWN,
        C_PURPLE, C_LTGREEN, C_LTBLUE, C_CYAN,
        C_ORANGE, C_YELLOW, C_TAN, C_PINK
    };
    if (dh_init(&config)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    srand(getpid());

    printf("dh_fill\n");
    printf("black\n");
    dh_fill(C_BLACK);
    sleep(DELAY);

    printf("dh_rectangle %d colors\n", sizeof(colors)/sizeof(uint16_t));
    for (i=0 ; i<sizeof(colors)/sizeof(uint16_t) ; i++) {
        dh_rectangle((i%4)*config.width/4,
                     (i/4)*config.height/4,
                     ((i%4)+1)*config.width/4,
                     (1+i/4)*config.height/4,
                     colors[i],
                     1);
        dh_display();
        sleep(DELAY);
    }

    printf("dh_rectangle\n");
    dh_fill(C_BLACK);

    r[0] = config.width ;
    r[1] = config.height ;
    for (i=0 ; i<100 ; i++) {
        randomize(r, &color);
        dh_rectangle(r[2], r[3], r[4], r[5], color, 0);
        randomize(r, &color);
        dh_rectangle(r[2], r[3], r[4], r[5], color, 1);
        dh_display();
    }
    sleep(DELAY);

    printf("dh_line\n");
    dh_fill(C_BLACK);
    for (i=0 ; i<100 ; i++) {
        randomize(r, &color);
        dh_line(r[2], r[3], r[4], r[5], color);
        dh_display();
    }
    sleep(DELAY);

    printf("dh_putpix\n");
    dh_fill(C_BLACK);
    for (i=0 ; i<200 ; i++) {
        randomize(r, &color);
        dh_putpix(r[2], r[3], color);
        dh_display();
    }
    sleep(DELAY);
    dh_fill(C_BLACK);
    return 0;
}
