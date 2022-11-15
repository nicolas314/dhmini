#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>

#include "dhmini.h"
#include "text.h"
#include "console.h"

#define FILENAME_SZ     1024
#define FONT_PATH       "/usr/share/fonts/truetype/aenigma"

char * pick_font(char * dirname)
{
    struct dirent * dir ;
    DIR * d ;
    int   nfiles ;
    int   i, r, len ;
    static char  filename[FILENAME_SZ+1];

    d = opendir(dirname);
    if (d==NULL) {
        printf("cannot open: %s\n", dirname);
        return NULL ;
    }
    /* Count number of font files in dir */
    nfiles=0 ;
    while ((dir=readdir(d))!=NULL) {
        len=strlen(dir->d_name);
        if (!strncmp(dir->d_name+len-4, ".ttf", 4)) {
            nfiles++;
        }
    }
    closedir(d);
    d = opendir(dirname);
    if (d==NULL) {
        printf("cannot open: %s\n", dirname);
        return NULL ;
    }
    r = rand() % nfiles ;
    i=0 ;
    while ((dir=readdir(d))!=NULL) {
        len=strlen(dir->d_name);
        if (!strncmp(dir->d_name+len-4, ".ttf", 4)) {
            if (i==r) {
                snprintf(filename, FILENAME_SZ, "%s/%s", dirname, dir->d_name);
                break;
            }
            i++ ;
        }
    }
    closedir(d);
    return filename ;
}

void pick_color(uint8_t * color)
{
    color[0] = 127 + rand() % 128;
    color[1] = 127 + rand() % 128;
    color[2] = 127 + rand() % 128;
}


int main(int argc, char *argv[]) {
    int width, height ;
    uint8_t  color[3];
    char * fontname ;

    if (argc<2) {
        printf("use: %s text [font]\n", argv[0]);
        return -1;
    }
    srand(getpid());
    if (argc==3) {
        fontname = argv[2];
    } else {
        fontname = pick_font(FONT_PATH);
    }
    pick_color(color);
    if (dh_init(NULL)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    dh_get_size(&width, &height);
    dh_text(argv[1],
            fontname,
            color,
            0,
            0,
            width,
            height);
    dh_printf(C_WHITE, basename(fontname));
    dh_display();
    return 0;
}

