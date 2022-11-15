#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include "dhmini.h"
#include "console.h"

#define PACK_RGB(r,g,b) (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3)

uint16_t pick_color(void)
{
    uint8_t r, g, b ;
    r = 127 + rand() % 128;
    g = 127 + rand() % 128;
    b = 127 + rand() % 128;
    return PACK_RGB(r,g,b);
}


#define LINE_SZ     40 

int main(int argc, char *argv[])
{
    FILE * in ;
    uint16_t * buf ;
	int i, j ;
	char * text ;
    uint16_t color ;
    char line[LINE_SZ];
    int sz=8 ;

    if (argc<2) {
        printf("use: %s FILE\n", argv[0]);
        return -1;
    }
    if (argc>2) sz=16 ;
    srand(getpid());

    if (dh_init(NULL)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    if ((in=fopen(argv[1], "r"))==NULL) {
        printf("cannot open: %s\n", argv[1]);
        return -1 ;
    }
    memset(line, 0, LINE_SZ);
    while (fgets(line, LINE_SZ, in)) {
        color = C_WHITE ;
        /* color = pick_color() ; */
        /* dh_print8x16(line, i, color); */
        dh_printf(color, line);
        dh_display();
    }
    fclose(in);

    return 0;
}

