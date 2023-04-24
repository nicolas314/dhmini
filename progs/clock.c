#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include "dhmini.h"
#include "truetype.h"
#include "buttons.h"
#include "console.h"
#include "random.h"

#define FILENAME_SZ     1024
#define FONT_PATH		"/opt/clock/display"

char * fontname = NULL ;

char * pick_font(char * dirname)
{
    struct dirent * dir ;
    DIR * d ;
    int   nfiles ;
    int   i, r, len ;
    static char  filename[FILENAME_SZ+1];

    // printf("picking font from [%s]\n", dirname);
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
    r = get_random() % nfiles ;
    // printf("files: %d picked: %d\n", nfiles, r);
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
    color[0] = 127 + get_random() % 128 ;
    color[1] = 127 + get_random() % 128 ;
    color[2] = 127 + get_random() % 128 ;
}


#define DATEBAR_SZ  40

void show_clock(void)
{
    int width, height ;
    uint8_t  color[3] = { 0, 0, 0 };
    char day[DATE_SZ], hh[3], mm[3];

    if (!fontname) {
        fontname = pick_font(FONT_PATH);
    }

    dh_get_size(&width, &height);
    date_now(day, hh, mm);
    random_colors(color);
    dh_fill(C_BLACK);
    dh_text(day,
            fontname,
            color,
            0,
            2,
            width,
            DATEBAR_SZ);

    random_colors(color);
    dh_text(hh,
            fontname,
            color,
            0,
            DATEBAR_SZ+30,
            width/2,
            height-DATEBAR_SZ-30);
    random_colors(color);
    dh_text(mm,
            fontname,
            color,
            width/2,
            DATEBAR_SZ+10,
            width/2,
            height-DATEBAR_SZ-10);
    dh_display();

    fontname=NULL ;
    return ;
}

void backlight_toggle(void)
{
    static int lit=1 ;
    lit = 1-lit ;
    dh_backlight_set(lit);
    return ;
}

int main(int argc, char *argv[]) {
    time_t rawtime ;
    struct tm * timeinfo ;
    int daemon=0 ;
    int c ;

    while ((c=getopt(argc, argv, "f:d"))!=EOF) {
        switch (c) {
            case 'f':
            fontname = optarg ;
            break;

            case 'd':
            daemon=1;
            break;

            default:
            printf("use: %s [-f font] [-d]\n", argv[0]);
            return -1;
        }
    }
    if (dh_init(NULL)!=0) {
        printf("cannot initialize display: aborting\n");
        return -1 ;
    }

    if (!daemon) {
        show_clock();
        return 0;
    }
    dh_button_assign("A", backlight_toggle);
    dh_button_assign("B", show_clock);
    if (dh_button_start()!=0) {
        printf("cannot start button listener\n");
    }

    /* Do it again every minute until killed */
    while (1) {
        show_clock();
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

