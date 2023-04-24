#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "image.h"
#include "dhmini.h"
#include "httpget.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

typedef struct {
    int w, h ;
    int channels ;
    uint8_t * pix ;
} image_t ;

static image_t * image_load(char * filename)
{
    FILE * in;
    image_t * im ;

    im = malloc(sizeof(image_t));
    im->pix = stbi_load(filename, &(im->w), &(im->h), &(im->channels), 3);
    return im ;
}

static image_t * image_download(char * url)
{
    image_t * im ;
    char    * buf ;
    size_t    sz ;

    buf = http_get(url, &sz);
    if (!buf || sz<1) {
        printf("failed to load [%s]\n", url);
        return NULL;
    }
    im = malloc(sizeof(image_t));
    im->pix = stbi_load_from_memory(buf, sz, &(im->w), &(im->h), &(im->channels), 3);
    if (!im->pix || im->w<1 || im->h<1) {
        printf("failed to load [%s]\n", url);
        free(im);
        im=NULL;
    }
    return im;
}

static image_t * image_resize(image_t * in, int width, int height)
{
    image_t * out ;
    uint8_t * outbuf ;

    if (!in) return NULL ;

    out = malloc(sizeof(image_t));
    out->w = width ;
    out->h = height ;
    out->channels = in->channels ;
    out->pix = malloc(width * height * out->channels);

    stbir_resize_uint8(in->pix, in->w, in->h, 0,
                       out->pix, out->w, out->h, 0,
                       in->channels);
    return out ;
}

static void image_free(image_t * im)
{
    if (!im) return ;
    if (im->pix) free(im->pix);
    free(im);
    return ;
}

#define RGB565_FROM_RGB(r, g, b)    (((uint16_t)r & 0xF8) << 8) | (((uint16_t)g & 0xFC) << 3) | ((uint16_t)b >> 3)
int dh_image(char * filename)
{
    image_t * in=NULL ;
    image_t * resized=NULL ;
    uint16_t * dh_buf ;
    int w, h ;
    int i ;
    uint8_t r, g, b ;

    if (!filename) return -1;
    
    if (!strncmp(filename, "http", 4)) {
        in = image_download(filename);
    } else {
        in = image_load(filename);
    }
    if (!in) return -1 ;
    dh_get_size(&w, &h);
    if (in->w != w || in->h != h) {
        resized = image_resize(in, w, h);
        image_free(in);
        in = resized ;
    }
    dh_buf = dh_frame_get();
    for (i=0 ; i<(w * h) ; i++) {
        r = in->pix[3*i];
        g = in->pix[3*i+1];
        b = in->pix[3*i+2];
        dh_buf[i] = RGB565_FROM_RGB(r, g, b);
    }
    image_free(in);
    dh_display();
    return 0 ;
}

