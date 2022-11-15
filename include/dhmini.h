/*
 * dhmini.h
 * Driver for Pimoroni display hat mini
 * Including this file gives you access to drawing primitives,
 * image, and character display.
 */

#ifndef _DHMINI_H_
#define _DHMINI_H_

#include <stdint.h>

/* Default sizes for display */
#define DHMINI_WIDTH    320
#define DHMINI_HEIGHT   240

/* Some RGB565 colours defined for convenience */
#define C_RED       0xf800
#define C_YELLOW    0xffe0
#define C_BLUE      0x001f
#define C_GREEN     0x0400
#define C_WHITE     0xffff
#define C_BLACK     0x0000

/*
 * Configuration for the dhmini object passed to dh_init()
 * Only needed if you intend to change from defaults
 */
typedef struct {
    char *  spi_path ;
    int     spi_speed ;
    char *  gpio_chip_name ;
    int     dc_pin ;
    int     bl_pin ;
    int     width ;
    int     height ;
} dh_config_t ;

/*
 * Init function
 * Pass NULL to use the default configuration.
 * Or fill up a dh_config_t structure with appropriate
 * values and pass a pointer to it.
 */
int dh_init(dh_config_t * config);
/*
 * Close function
 * Registered with atexit()
 */
void dh_close(void);

/*
 * Get frame size in pixels once initialized
 */
void dh_get_size(int *h, int *w);

/*
 * Get pointer to raw frame as 16-bit
 * Get the buffer dimensions with dh_get_size()
 * Pixels are 16-bit RGB 565 values.
 */
uint16_t * dh_frame_get(void);

/*
 * Reset the current frame buffer with zeros
 */
void dh_clear(void);

/*
 * Copy the current frame buffer to the ST7789 display
 * through SPI.
 */
void dh_display(void);

/*
 * Restrict display area to a rectangle
 */
void dh_set_display_area(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1);

/* Set a single pixel in the frame buffer */
void dh_putpix(
    uint16_t x,
    uint16_t y,
    uint16_t color);

/*
 * Copy the passed buffer into the frame buffer
 */
void dh_frame_set(uint16_t * buf);

/*
 * Pack an 8/8/8 RGB frame into the frame buffer
 * The buffer must have the same width and height as
 * the screen and pack pixels as RGB 888 bits.
 */
void dh_frame_set_rgb(uint8_t * buf);

/*
 * Fill the whole frame buffer with that color
 */
void dh_fill(uint16_t color);

/* Draw a line in the frame buffer */
void dh_line(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint16_t color);

/* Draw a rectangle in the frame buffer */
void dh_rectangle(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint16_t color,
    int filled);

#endif
