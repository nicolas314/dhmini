#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "dhmini.h"
#include "image.h"

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("use: %s IMAGE\n", argv[0]);
        return -1;
    }
    if (dh_init(NULL)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    dh_image(argv[1]);
    return 0;
}

