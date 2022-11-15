#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dhmini.h"
#include "console.h"

#define LINE_SZ 64

int main(int argc, char * argv[])
{
    char line[LINE_SZ+1];

    if (dh_init(0)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    while (fgets(line, LINE_SZ, stdin)>0) {
        dh_printf(C_WHITE, "%s", line);
        dh_display();
        printf("%s", line);
    }
    return 0 ;
}

