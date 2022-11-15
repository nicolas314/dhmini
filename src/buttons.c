#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <gpiod.h>

#define GPIO_CHIP_NAME  "gpiochip0"

#define N_BUTTONS   4

#define PIN_A   5
#define PIN_B   6
#define PIN_X  16
#define PIN_Y  24

struct {
    int pin ;
    struct gpiod_line * line ;
    char * name ;
} dh_buttons[] = {
    {PIN_A, NULL, "A"},
    {PIN_B, NULL, "B"},
    {PIN_X, NULL, "X"},
    {PIN_Y, NULL, "Y"}
};
struct gpiod_chip * dh_button_chip ;

int dh_button_init(void)
{
    char * gpio_chip_name ;
    int i ;
    int r ;

    dh_button_chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME); 
    if (!dh_button_chip) {
        printf("cannot open %s\n", GPIO_CHIP_NAME);
        return -1 ;
    }
    /* Get a line per button */
    for (i=0 ; i<N_BUTTONS; i++) {
        // printf("init[%d][%s][%d]\n", i, dh_buttons[i].name, dh_buttons[i].pin);
        dh_buttons[i].line = gpiod_chip_get_line(dh_button_chip, dh_buttons[i].pin);
        if (dh_buttons[i].line==NULL) {
            printf("cannot get line: %d\n", i);
            perror("gpiod_get_line");
            return -1 ;
        }
        r = gpiod_line_request_both_edges_events(dh_buttons[i].line, "dhbuttons");
        if (r!=0) {
            printf("failed to request edge events for [%s](%d)\n", dh_buttons[i].name, r);
            perror("gpiod_line_request_both_edges_events");
            return -1;
        }
    }
    return 0 ;
}

void dh_button_close(void)
{
    int i ;
    for (i=0 ; i<N_BUTTONS ; i++) {
        gpiod_line_release(dh_buttons[i].line);
        dh_buttons[i].line=NULL ;
    }
    gpiod_chip_close(dh_button_chip);
    return ;
}

void dh_button_pressed(int bid)
{
    printf("pressed[%s]\n", dh_buttons[bid].name);
    switch (bid) {
        case 0: // A
        break;
        case 1: // B
        break;
        case 2: // X
        break;
        case 3: // Y
        break;
        default:
        break;
    }
    return ;
}

void * dh_event(void * arg)
{
    int button_id ;
    struct gpiod_line_event event;

    button_id = *(int*)arg ;
    printf("waiting on %d[%s]\n", button_id, dh_buttons[button_id].name);
    while (1) {
        //printf("waiting on line %p\n", dh_buttons[button_id].line);
        gpiod_line_event_wait(dh_buttons[button_id].line, NULL);
        if (gpiod_line_event_read(dh_buttons[button_id].line, &event)==0) {
            if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                dh_button_pressed(button_id);
                break;
            }
        }
    }
    return 0;

}

int main(int argc, char * argv[])
{
    int i;
    pthread_t thread_id[N_BUTTONS];

    printf("init GPIO\n");
    if (dh_button_init()!=0) {
        printf("failed init\n");
        return -1;
    }
    
    printf("starting threads\n");
    for (i=0 ; i<N_BUTTONS ; i++) {
        printf("starting thread [%d][%s]\n", i, dh_buttons[i].name);
        pthread_create(&(thread_id[i]), NULL, &dh_event, (void*)&i);
        usleep(100000);
    }
    printf("waiting on threads\n");
    for (i=0 ; i<N_BUTTONS ; i++) {
        pthread_join(thread_id[i], NULL);
    }
    printf("closing down\n");
    dh_button_close();
	return 0 ;
}


