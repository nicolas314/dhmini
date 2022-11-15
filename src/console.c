#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "dhmini.h"
#include "font8x16.h"

#define bitval(c,n) (((c)>>(n))&1)

/*
 * Print char on screen at position x,y
 * Expects an 8x16 font on a 320x240 screen
 * x from 0 to 39, y from 0 to 15
 */
static int dh_char8x16(char c, int x, int y, uint16_t color)
{
    int i, b ;
    uint8_t * p ;

    p = seabios8x16 + c*16 ;
    for (i=0 ; i<16 ; i++) {
        for (b=7 ; b>=0 ; b--) {
            if (bitval(p[i], b)) {
                dh_putpix(x*8+7-b, y*16+i, color); 
            }
        }
    }
    /* dh_rectangle(8*x, 16*y, 8*x+8, 16*y+16, C_RED, 0); */
    return 0;
}

static void dh_line8x16(char * text, int line, uint16_t color)
{
    int i ;
    for (i=0 ; i<(int)strlen(text) ; i++) {
        if (text[i]=='\n')
            return ;
        dh_char8x16(text[i], i, line, color);
    }
}

#define LINE_SZ     1024
void dh_printf(uint16_t color, char * fmt, ...)
{
    static int line_num=0;
    int        line_max ;
    int w, h ;
    uint16_t * frame_plus ;
    char       line[LINE_SZ+1];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, LINE_SZ, fmt, ap);
    va_end(ap);

    line_num++ ;
    dh_get_size(&w, &h);
    line_max = (h/16) ;

    if (line_num>=line_max) {
        /* Move screen up by 16 pixels */
        frame_plus = calloc(w*(h+16), sizeof(uint16_t));
        memcpy(frame_plus, dh_frame_get()+w*16, w*(h-16)*2);
        dh_frame_set(frame_plus);
        free(frame_plus);
        line_num = line_max-1 ;
    }
    dh_line8x16(line, line_num, color);
    return ; 
}

