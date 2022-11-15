#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include "dhmini.h"
#include "text.h"

#define FILENAME_SZ     1024
#define FONT_PATH		"/opt/clock/display"

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


#define DATE_SZ 32
void date_now(char * day, char * hh, char * mm)
{
    time_t rawtime ;
    struct tm * timeinfo ;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (day)
        strftime(day, DATE_SZ, "%A %d %b", timeinfo);
    if (hh)
        strftime(hh, 3, "%H", timeinfo);
    if (mm)
        strftime(mm, 3, "%M", timeinfo);
}

void random_colors(uint8_t * color)
{
    color[0] = 127 + rand() % 128 ;
    color[1] = 127 + rand() % 128 ;
    color[2] = 127 + rand() % 128 ;
}


#define DATEBAR_SZ  40

int show_clock(char * fontname)
{
    int width, height ;
    uint16_t buf[DHMINI_WIDTH*DHMINI_HEIGHT];
    uint8_t  color[3] = { 0, 0, 0 };
    char day[DATE_SZ], hh[3], mm[3];

    srand((unsigned int)time(NULL));
    if (!fontname)
        fontname = pick_font(FONT_PATH);
    dh_get_size(&width, &height);
    memset(buf, 0, sizeof(buf));
    date_now(day, hh, mm);
    random_colors(color);
    dh_text(day,
            fontname,
            color,
            0,
            0,
            width,
            DATEBAR_SZ);

    random_colors(color);
    dh_text(hh,
            fontname,
            color,
            0,
            DATEBAR_SZ,
            width/2,
            height-DATEBAR_SZ);
    random_colors(color);
    dh_text(mm,
            fontname,
            color,
            width/2,
            DATEBAR_SZ,
            width/2,
            height-DATEBAR_SZ);
    dh_frame_set(buf);
    dh_display();

    return 0;
}

int main(int argc, char *argv[]) {
	char * fontname ;
    time_t rawtime ;
    struct tm * timeinfo ;

    srand((unsigned int)time(NULL));
    if (argc>1) {
        fontname = argv[1];	
    } else {
        fontname = pick_font(FONT_PATH);
    }
    if (dh_init(NULL)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }
    while (1) {
        show_clock(fontname);
        while (1) {
            sleep(1);
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            if (timeinfo->tm_sec<1)
                break;
        }
    }
    return 0;
}

