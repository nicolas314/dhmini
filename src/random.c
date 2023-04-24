#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned get_random(void)
{
    FILE * f ;
    unsigned r ;

    if ((f=fopen("/dev/urandom", "r"))==NULL) {
        return 0 ;
    }
    fread(&r, 1, sizeof(int), f);
    fclose(f);
    return r;
}
