#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "dhmini.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library library ;

#define PACK_RGB(r,g,b) (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3)

static void draw_bitmap(
    FT_Bitmap*  bitmap,
    FT_Int      x,
    FT_Int      y,
    uint16_t *  buf,
    int         lx,
    int         ly,
    int         r,
    int         g,
    int         b)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;
    float    dark ;
    uint8_t  r8, g8, b8 ;
    uint16_t pix ;

    /*
    printf("draw at  [%d %d]\n", x, y);
    printf("glyph sz [%d %d]\n", bitmap->width, bitmap->rows);
    */

    for (i=x, p=0; i<x_max; i++, p++) {
        for (j=y, q=0; j<y_max; j++, q++) {
            if (i<0 || j<0 || i>=lx || j>=ly)
                continue;
            dark = (float)bitmap->buffer[q*bitmap->width+p]/255.0;
            r8 = (uint8_t)((float)r * dark);
            g8 = (uint8_t)((float)g * dark);
            b8 = (uint8_t)((float)b * dark);
            pix = PACK_RGB(r8, g8, b8);
            buf[i+j*lx] |= pix ;
        }
    }
}

#define MAXVAL(x,y) (((x)>(y)) ? (x) : (y))

/*
 * Fit text into rectangle of size width*height
 */
static int fit(
    FT_Face face,
    char * text,
    int width,
    int height)
{
    FT_UInt gi ;
    FT_Error error ;
    int x_off, y_off ;
    int i ;
    int sz ;

    for (sz=4 ; sz<1000 ; sz++) {
        error = FT_Set_Pixel_Sizes(face, 0, sz);
        x_off = 0;
        y_off = 0;
        for (i=0 ; i<strlen(text) ; i++) {
            gi = FT_Get_Char_Index(face, text[i]);
            FT_Load_Glyph(face, gi, FT_LOAD_NO_BITMAP);
            x_off+=face->glyph->metrics.horiAdvance ;
            y_off =MAXVAL(face->glyph->metrics.height, y_off);
        }
        x_off >>= 6;
        y_off >>= 6;
        if (x_off>width || y_off>height) {
            return sz-1 ;
        }
    }
    return 0 ;
}


int dh_text(
    char * text,
    char * fontname,
    uint8_t color[3],
    int at_x,
    int at_y,
    int size_x,
    int size_y)
{
    FT_Face face ;
    FT_Vector pen ;
    FT_GlyphSlot slot ;
    FT_Error error ;
    static int lib_init=0 ;
    int sz, n ;
    uint16_t *  dh_frame;
    int         dh_width, dh_height ;


    if (!lib_init) {
        FT_Init_FreeType(&library);
        lib_init++;
    }
    /* Fit text into box */
    error = FT_New_Face(library, fontname, 0, &face);
    if (error) {
        printf("cannot load: %s\n", fontname);
        return -1;
    }
    sz = fit(face, text, size_x, size_y);
    FT_Set_Pixel_Sizes(face, 0, sz);
    slot = face->glyph ;

    pen.x = at_x ;
    pen.y = at_y ;

    dh_get_size(&dh_width, &dh_height);
    dh_frame = dh_frame_get() ;

    for (n=0 ; n<(int)strlen(text) ; n++) {
        FT_Load_Char(face, text[n], FT_LOAD_RENDER);
        draw_bitmap(
            &slot->bitmap,
            pen.x,
            pen.y+size_y-slot->bitmap_top-size_y/4,
            dh_frame,
            dh_width,
            dh_height,
            color[0], color[1], color[2]);
        pen.x += slot->advance.x>>6;
        pen.y += slot->advance.y>>6;
    }
    FT_Done_Face(face);
}

