#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <png.h>
#include <jpeglib.h>

#include "image.h"
#include "dhmini.h"

#define PNG_HEADER_SZ   8

#define PACK_RGB(r,g,b) (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3)

int filesize(char *filename)
{
    int size ;
    struct stat fileinfo ;
    /* POSIX compliant  */
    if (stat(filename, &fileinfo) != 0) {
        size = (int)0 ;
    } else {
        size = (int)fileinfo.st_size ;
    }
    return size ;
}

static int dh_image_raw(char * filename)
{
    FILE * fp ;
    uint16_t buf[DHMINI_WIDTH * DHMINI_HEIGHT];
    if ((fp=fopen(filename, "r"))==NULL) {
        printf("cannot open: %s\n", filename);
        return -1;
    }
    fread(buf, sizeof(uint16_t), sizeof(buf), fp);
    fclose(fp);
    dh_frame_set(buf);
	return 0 ;
}


static int dh_image_png(char * filename)
{
    FILE * f_in ;
    unsigned char header[PNG_HEADER_SZ];
    png_structp png_ptr ;
    png_infop info_ptr ;
    int width, height;
    int bit_depth, color_type, interlace_type, channels, rowbytes ;
    png_bytepp row_pointers ;
    int i, j, n ;
    uint16_t buf[DHMINI_WIDTH * DHMINI_HEIGHT];
    uint16_t   color ;
    int w, h ;

    f_in = fopen(filename, "r");
    if (!f_in) {
        printf("cannot read: %s\n", filename);
        return -1;
    }
    fread(header, 1, PNG_HEADER_SZ, f_in);
    if (png_sig_cmp(header, 0, PNG_HEADER_SZ)!=0) {
        printf("not a PNG file: %s\n", filename);
        fclose(f_in);
        return -1;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, f_in);
    png_set_sig_bytes(png_ptr, PNG_HEADER_SZ);
    png_read_info(png_ptr, info_ptr);

    width       = png_get_image_width(png_ptr, info_ptr);
    height      = png_get_image_height(png_ptr, info_ptr);
    bit_depth   = png_get_bit_depth(png_ptr, info_ptr);
    color_type  = png_get_color_type(png_ptr, info_ptr);
    interlace_type = png_get_interlace_type(png_ptr, info_ptr);
    channels    = png_get_channels(png_ptr, info_ptr);
    rowbytes    = png_get_rowbytes(png_ptr, info_ptr);

    dh_get_size(&w, &h);
	if (width!=w || height!=h) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(f_in);
        printf("cannot load %s [%dx%d]\n", filename, width, height);
        return -1 ;

    }
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
    if (color_type & PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha(png_ptr);

    row_pointers = (png_bytepp)malloc(sizeof(png_bytepp) * height);
    for (i = 0; i < height; i++) {
        row_pointers[i] = (png_bytep)malloc(rowbytes);
    }
    png_read_image(png_ptr, row_pointers);
    n=0 ;
    for (j=0 ; j<height ; j++) {
        for (i=0 ; i<width ; i++) {
            color = PACK_RGB(row_pointers[j][3*i],
                             row_pointers[j][3*i+1],
                             row_pointers[j][3*i+2]);
            buf[n]=color;
            n++;
        }
    }
    fclose(f_in);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    dh_frame_set(buf);
    return 0;
}

static int dh_image_jpg(char * filename)
{
    FILE * f_in ;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;            /* Output row buffer */
    int row_stride;               /* physical row width in output buffer */

    int i, j, n ;
    int width, height ;
    uint16_t * buf ;
    uint16_t   color ;
    int w, h ;

    f_in = fopen(filename, "r");
    if (!f_in) {
        printf("cannot read: %s\n", filename);
        return -1;
    }

    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_stdio_src(&cinfo, f_in);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    width = cinfo.output_width ;
    height = cinfo.output_height ;

    dh_get_size(&w, &h);
	if (width!=w || height!=h) {
        printf("cannot read %s size[%dx%d]\n", filename, width, height);
        fclose(f_in);
        jpeg_destroy_decompress(&cinfo);
        return -1 ;
    }
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    n=0 ;
    buf = malloc(width*height*sizeof(uint16_t));
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (i=0 ; i<width ; i++) {
            color = PACK_RGB(buffer[0][3*i],
                             buffer[0][3*i+1],
                             buffer[0][3*i+2]);
            buf[n]=color;
            n++;
        }
    }
    fclose(f_in);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    dh_frame_set(buf);
    free(buf);
    return 0;
}

static int dh_image_rgb(char * filename)
{
    FILE    * in ;
    uint8_t * buf ;
    int       sz ;

    sz = filesize(filename);
    buf = malloc(sz);
    if ((in=fopen(filename, "rb"))==NULL) {
        return -1;
    }
    fread(buf, sz, 1, in);
    fclose(in);
    dh_frame_set_rgb(buf);
    free(buf);
    return 0;
}

int dh_image(char * filename)
{
    int len ;
    int ret ;
    len = (int)strlen(filename);
    if (!strncmp(filename+len-4, ".raw", 4)) {
        ret=dh_image_raw(filename);
    } else if (!strncmp(filename+len-4, ".rgb", 4)) {
        ret=dh_image_rgb(filename);
    } else if (!strncmp(filename+len-4, ".png", 4)) {
        ret=dh_image_png(filename);
    } else if (!strncmp(filename+len-4, ".jpg", 4)) {
        ret=dh_image_jpg(filename);
    }
    if (!ret)
        dh_display();
    return ret ;
}

