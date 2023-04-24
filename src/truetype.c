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

// https://kevinboone.me/fbtextdemo.html?i=1

/*
 * Fit text into rectangle of size width*height
 */
#define MAXVAL(x,y) (((x)>(y)) ? (x) : (y))
static int fit(
    FT_Face face,
    char * text,
    int width,
    int height,
    int * fit_x,
    int * fit_y)
{
    FT_UInt gi ;
    FT_Error error ;
    int x_off, y_off ;
    int i ;
    int sz ;

    for (sz=4 ; sz<8*height ; sz++) {
        error = FT_Set_Pixel_Sizes(face, 0, sz);
        x_off = 0;
        y_off = 0;
        for (i=0 ; i<strlen(text) ; i++) {
            gi = FT_Get_Char_Index(face, text[i]);
            FT_Load_Glyph(face, gi, FT_LOAD_NO_BITMAP);
            x_off+=face->glyph->metrics.horiAdvance ;
            //y_off =MAXVAL(face->glyph->metrics.height, y_off);
            y_off =MAXVAL(face->glyph->metrics.height+
                          face->glyph->metrics.vertBearingY, y_off);
        }
        x_off /= 64;
        y_off /= 64;
        if (x_off>width || y_off>height) {
            return sz-1 ;
        }
        *fit_x = x_off;
        *fit_y = y_off;
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
    FT_ULong character ;
    FT_UInt gi ;
    FT_Error error ;
    static int lib_init=0 ;
    int sz, n ;
    uint16_t *  dh_frame;
    int         dh_width, dh_height ;
	int	        to_x, to_y ;
    int         fit_x, fit_y ;
    float    dark ;
    uint8_t  r8, g8, b8 ;
    uint16_t pix ;


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
    sz = fit(face, text, size_x, size_y, &fit_x, &fit_y);
    at_x += (size_x-fit_x)/2;
    at_y += size_y/2 + (size_y-fit_y)/2 ;
    FT_Set_Pixel_Sizes(face, 0, sz);
    dh_get_size(&dh_width, &dh_height);
    dh_frame = dh_frame_get() ;

    for (n=0 ; n<(int)strlen(text) ; n++) {
        gi = FT_Get_Char_Index(face, (FT_ULong)text[n]);
        if (!gi) {
            gi = FT_Get_Char_Index(face, '?');
        }
        FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT);

        int glyph_width = face->glyph->metrics.width / 64 ;
        int advance = face->glyph->metrics.horiAdvance / 64 ;
        int x_off = (advance - glyph_width) / 2 ;
        int y_off = (face->bbox.yMax - face->glyph->metrics.horiBearingY) / 64 ;

        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		for (int i=0; i<(int)face->glyph->bitmap.rows; i++) {
			int row_offset = at_y + i + y_off;
			for (int j=0; j<(int)face->glyph->bitmap.width; j++) {
                to_x = at_x + j + x_off ;
                to_y = row_offset ;
                if (to_x<0 || to_x>=dh_width || to_y<0 || to_y>=dh_height)
                    continue;
				unsigned char p = face->glyph->bitmap.buffer[i*face->glyph->bitmap.pitch + j];
				if (p) {
					dark = (float)p/255.0;
					r8 = (uint8_t)((float)color[0] * dark);
					g8 = (uint8_t)((float)color[1] * dark);
					b8 = (uint8_t)((float)color[2] * dark);
					pix = PACK_RGB(r8, g8, b8);
					dh_frame[to_x + to_y * dh_width] = pix ;
				}
			}
		}
		at_x += advance;
    }
    FT_Done_Face(face);
}

