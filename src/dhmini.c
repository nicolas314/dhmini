
/*
 * dhmini: driver for Pimoroni Display Hat Mini
 * This driver uses the spidev framework from the Linux kernel
 * for communicating with the ST7789 display, and the GPIOd
 * framework for managing GPIOs: DC and Backlight.
 * If you want to receive button events, include "buttons.h".
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <gpiod.h>
#include <linux/spi/spidev.h>

#include "dhmini.h"

#define ERRMAX  255

/* Default setting for Pimoroni mini display hat */
#define PIMORONI_DHMINI_SPI_PATH        "/dev/spidev0.1"
#define PIMORONI_DHMINI_SPI_SPEED       80000000
#define PIMORONI_DHMINI_GPIO_CHIP       "gpiochip0"
#define PIMORONI_DHMINI_DC_PIN          9
#define PIMORONI_DHMINI_BACKLIGHT_PIN   13

static int st7789_initialized ;

struct {
    /* Configurable part */
    const char *spi_path;
    int spi_speed;
    const char *gpio_chip_name;
    int dc_pin;
    int bl_pin ;
    uint16_t width;
    uint16_t height;

    /* Internal */
    int spi_fd;
    struct gpiod_chip * gpio_chip ;
    struct gpiod_line * gpio_dc;
    struct gpiod_line * gpio_backlight;
    uint16_t * frame ;
    size_t     frame_sz ;
} dhmini ;


/* ST7789 commands */
typedef enum {
    ST_NOP      = 0x00, // NOP
    ST_SWRESET  = 0x01, // Software reset
    ST_RDID     = 0x04, // Read display ID
    ST_RDDST    = 0x09, // Read display status
    ST_RDDPM    = 0x0a, // Read display power mode
    ST_RDDMADCTL= 0x0b, // Read display MADCTL
    ST_RDDCOLMOD= 0x0c, // Read display pixel format
    ST_RDDIM    = 0x0d, // Read display image mode
    ST_RDDSM    = 0x0e, // Read display signal mode
    ST_RDDSDR   = 0x0f, // Read display self-diagnostic result

    ST_SLPIN    = 0x10, // Sleep in
    ST_SLPOUT   = 0x11, // Sleep out
    ST_PTLON    = 0x12, // Partial display mode on
    ST_NORON    = 0x13, // Normal display mode on

    ST_INVOFF   = 0x20, // Display inversion off
    ST_INVON    = 0x21, // Display inversion on
    ST_GAMSET   = 0x26, // Gamma set
    ST_DISPOFF  = 0x28, // Display off
    ST_DISPON   = 0x29, // Display on
    ST_CASET    = 0x2a, // Column address set
    ST_RASET    = 0x2b, // Row address set
    ST_RAMWR    = 0x2c, // Memory write
    ST_RAWRD    = 0x2e, // Memory read

    ST_PTLAR    = 0x30, // Partial area

    ST_VSCRDEF  = 0x33, // Vertical scrolling definition
    ST_TEOFF    = 0x34, // Tearing effect line off
    ST_TEON     = 0x35, // Tearing effect line on
    ST_MADCTL   = 0x36, // Memory Data Access Control
    ST_VSCSAD   = 0x37, // Vertical scroll start address of RAM
    ST_IDMOFF   = 0x38, // Idle mode off
    ST_IDMON    = 0x39, // Idle mode on
    ST_COLMOD   = 0x3a, // Interface pixel format
    ST_WRMEM    = 0x3c, // Write memory continue
    ST_RDMEM    = 0x3e, // Read memory continue

    ST_STE      = 0x44, // Set tear scanline
    ST_GSCAN    = 0x45, // Get scanline

    ST_WRDISBV  = 0x51, // Write display brightness
    ST_RDDISBV  = 0x52, // Read display brightness
    ST_WRCTRLD  = 0x53, // Write CTRL display
    ST_RDCTRLD  = 0x54, // Read CTRL value display
    ST_WRCACE   = 0x55, // Write content adaptive bright control and color enhancement
    ST_RDCABC   = 0x56, // Read content adaptive brightness control
    ST_WRCABCMB = 0x5e, // Write CABC minimum brightness
    ST_RDCABCMB = 0x5f, // Read CABC minimum brightness

    ST_FRMCTR1  = 0xb1, //
    ST_FRMCTR2  = 0xb2, //
    ST_FRMCTR3  = 0xb3, //
    ST_INVCTR   = 0xb4, //
    ST_DISSET5  = 0xb6, //
    ST_GCTRL    = 0xb7, //
    ST_GTADJ    = 0xb8, //
    ST_VCOMS    = 0xbb, //

    ST_LCMCTRL  = 0xc0, //
    ST_IDSET    = 0xc1, //
    ST_VDVVRHEN = 0xc2, //
    ST_VRHS     = 0xc3, //
    ST_CDVS     = 0xc4, //
    ST_VMCTR1   = 0xc5, //
    ST_FRCTRL2  = 0xc6, //
    ST_CABCCTRL = 0xc7, //

    ST_PWCTRL1  = 0xd0, // Power Control 1
    ST_RDID1    = 0xda, // Read ID1
    ST_RDID2    = 0xdb, // Read ID2
    ST_RDID3    = 0xdc, // Read ID3
    ST_RDID4    = 0xdd, //

    ST_GMCTRP1  = 0xe0, //
    ST_GMCTRN1  = 0xe1, //

    ST_PWCTR6   = 0xfc, //

    ST_INVALID  = 0xff
} st7789_cmd ;


#define SPI_CMD 0
#define SPI_DAT 1

#define SWAP16(u)   (((u)>>8) | ((u)<<8))

static uint8_t st7789_init_seq[] = {
    ST_MADCTL,      1, 0x60,
    /* ST_MADCTL,      1, 0x00, */
    ST_COLMOD,      1, 0x05,
    ST_FRMCTR2,     5, 0x0c, 0x0c, 0x00, 0x33, 0x33,
    ST_GCTRL,       1, 0x35,
    ST_VCOMS,       1, 0x19,
    ST_LCMCTRL,     1, 0x2c,
    ST_VDVVRHEN,    1, 0x01,
    ST_VRHS,        1, 0x12,
    ST_CDVS,        1, 0x20,
    ST_FRCTRL2,     1, 0x0f,
    ST_PWCTRL1,     2, 0xa4, 0xa1,
    ST_GMCTRP1,    14, 0xd0, 0x04, 0x0d, 0x11, 0x13, 0x2b, 0x3f,
                       0x54, 0x4c, 0x18, 0x0d, 0x0b, 0x1f, 0x23,
    ST_GMCTRN1,    14, 0xd0, 0x04, 0x0c, 0x11, 0x13, 0x2c, 0x3f,
                       0x44, 0x51, 0x2f, 0x1f, 0x1f, 0x20, 0x23,
    ST_INVON,       0,
    ST_SLPOUT,      0,
    ST_DISPON,      0,
    ST_MADCTL,      1, 0x60,
    /* ST_MADCTL,      1, 0x00, */
    ST_INVALID
};

static int dh_spi_write_8bit(int t, uint8_t data)
{
    struct spi_ioc_transfer spi = {
        .tx_buf = (unsigned long)&data,
        .rx_buf = 0,
        .delay_usecs = 0,
        .len = 1,
        .speed_hz = dhmini.spi_speed
    };
    /* Set type through GPIO */
    gpiod_line_set_value(dhmini.gpio_dc, t);
    if (ioctl(dhmini.spi_fd, SPI_IOC_MESSAGE(1), &spi)<0) {
        printf("ioctl failed (single 8bit)\n");
        return -1 ;
    }
    return 0 ;
}

static int dh_spi_write_16bit(uint16_t data)
{
    uint16_t swapped = SWAP16(data);
    struct spi_ioc_transfer spi = {
        .tx_buf = (unsigned long)&swapped,
        .rx_buf = 0,
        .delay_usecs = 0,
        .len = 2,
        .speed_hz = dhmini.spi_speed
	};
    gpiod_line_set_value(dhmini.gpio_dc, SPI_DAT);
    if (ioctl(dhmini.spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        printf("ioctl failed (single 16bit)\n");
		return -1 ;
	}
	return 0 ;
}

static int dh_spi_write_8bit_arr(uint8_t *data, uint16_t len)
{
    struct spi_ioc_transfer spi = {
        .tx_buf = (unsigned long)data,
        .rx_buf = 0,
        .delay_usecs = 0,
        .len = len,
        .speed_hz = dhmini.spi_speed
    };
    gpiod_line_set_value(dhmini.gpio_dc, SPI_DAT);
    if (ioctl(dhmini.spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        printf("ioctl failed (multi 8bit)\n");
		return -1 ;
	}
	return 0;
}

#define CHUNK_SZ    (1024)
static int dh_spi_write_16bit_arr(uint16_t *data, int count)
{
    int sent=0 ;
    int next_chunk ;
    int i ;
    uint16_t line[CHUNK_SZ] ;
    uint16_t * src ;

    if (count<CHUNK_SZ) {
        next_chunk = count ;
    } else {
        next_chunk = CHUNK_SZ ;
    }
    src=data ;
    while (sent<count) {
        /* Copy data into line and byteswap it */
        for (i=0 ; i<next_chunk ; i++) {
			line[i] = SWAP16(src[i]);
        }
		dh_spi_write_8bit_arr((uint8_t*)line, next_chunk * sizeof(uint16_t));
        sent+=next_chunk ;
        src+=next_chunk ;
        if (count-sent < CHUNK_SZ) {
            next_chunk = count-sent ;
        } else {
            next_chunk = CHUNK_SZ ;
        }
    }
    return 0;
}

void dh_close(void)
{
    st7789_initialized-- ;
    if (st7789_initialized<0)
        return ;
    gpiod_line_request_output(dhmini.gpio_backlight, "gpio_backlight", 0);
    if (dhmini.spi_fd) close(dhmini.spi_fd);
    if (dhmini.gpio_dc) gpiod_line_release(dhmini.gpio_dc);
    if (dhmini.gpio_dc) gpiod_line_release(dhmini.gpio_backlight);
    if (dhmini.gpio_chip) gpiod_chip_close(dhmini.gpio_chip);
    if (dhmini.frame) {
        free(dhmini.frame);
        dhmini.frame = NULL ;
        dhmini.frame_sz = 0 ;
    }
}

void dh_clear(void)
{
    memset(dhmini.frame, 0, dhmini.frame_sz);
}

void dh_set_display_area(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1)
{
    dh_spi_write_8bit(SPI_CMD, ST_CASET);
    dh_spi_write_16bit(x0);
    dh_spi_write_16bit(x1);

    dh_spi_write_8bit(SPI_CMD, ST_RASET);
    dh_spi_write_16bit(y0);
    dh_spi_write_16bit(y1);

    dh_spi_write_8bit(SPI_CMD, ST_RAMWR);
}


void dh_display(void)
{
    dh_set_display_area(0, 0, dhmini.width, dhmini.height);
    dh_spi_write_16bit_arr(dhmini.frame, dhmini.width * dhmini.height);
}

void dh_get_size(int * width, int * height)
{
    *width = dhmini.width ;
    *height = dhmini.height ;
    return ;
}

uint16_t * dh_frame_get(void)
{
    return dhmini.frame ;
}

int dh_init(dh_config_t * config)
{
    int     err=0;
    char    errmsg[ERRMAX+1];
    uint8_t bits=8,
            mode=SPI_MODE_2;
    int     i, j ;
    uint8_t cmd, len ;
    
    st7789_initialized=1 ;
    memset (&dhmini, 0, sizeof(dhmini));
    if (config) {
        dhmini.spi_path         = config->spi_path ;
        dhmini.spi_speed        = config->spi_speed ;
        dhmini.gpio_chip_name   = config->gpio_chip_name ;
        dhmini.dc_pin           = config->dc_pin ;
        dhmini.bl_pin           = config->bl_pin ;
        dhmini.width = (uint16_t)(config->width ? config->width : DHMINI_WIDTH) ;
        dhmini.height = (uint16_t)(config->height ? config->height : DHMINI_HEIGHT) ;
    } else {
        dhmini.spi_path         = PIMORONI_DHMINI_SPI_PATH;
        dhmini.spi_speed        = PIMORONI_DHMINI_SPI_SPEED;
        dhmini.gpio_chip_name   = PIMORONI_DHMINI_GPIO_CHIP;
        dhmini.dc_pin           = PIMORONI_DHMINI_DC_PIN;
        dhmini.bl_pin           = PIMORONI_DHMINI_BACKLIGHT_PIN;
        dhmini.width            = (uint16_t)DHMINI_WIDTH ;
        dhmini.height           = (uint16_t)DHMINI_HEIGHT ;
    }


    do {
        /* Open SPI device */
        if ((dhmini.spi_fd = open(dhmini.spi_path, O_NONBLOCK))<0) {
            sprintf(errmsg, "cannot open: %s", dhmini.spi_path);
            err++ ;
            break;
        }
        /* Set SPI bits */
        if ((ioctl(dhmini.spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits))<0) {
            sprintf(errmsg, "cannot set SPI bits");
            err++ ;
            break ;
        }
        /* Set SPI mode */
        if ((ioctl(dhmini.spi_fd, SPI_IOC_WR_MODE, &mode))<0) {
            sprintf(errmsg, "cannot set SPI mode");
            err++;
            break ;
        }
        /* Set SPI speed */
        if ((ioctl(dhmini.spi_fd, SPI_IOC_WR_MODE, &(dhmini.spi_speed)))<0) {
            sprintf(errmsg, "cannot set SPI speed");
            err++;
            break ;
        }

        /* Access GPIO chip by name */
        dhmini.gpio_chip = gpiod_chip_open_by_name(dhmini.gpio_chip_name);
        /* Set GPIO DC */
        dhmini.gpio_dc   = gpiod_chip_get_line(dhmini.gpio_chip, dhmini.dc_pin);
        if (!dhmini.gpio_dc) {
            sprintf(errmsg, "cannot set GPIO DC (line %d)", dhmini.dc_pin);
            err++;
            break ;
        }
        if ((gpiod_line_request_output(dhmini.gpio_dc, "gpio_dc", 0)) < 0) {
            sprintf(errmsg, "cannot set DC to output (line %d)", dhmini.dc_pin);
            err++ ;
            break ;
        }
        /* Set backlight */
        dhmini.gpio_backlight = gpiod_chip_get_line(dhmini.gpio_chip, dhmini.bl_pin);
        if (!dhmini.gpio_backlight) {
            sprintf(errmsg, "cannot set GPIO backlight (line %d)", dhmini.bl_pin);
            err++ ;
            break ;
        }
        if ((gpiod_line_request_output(dhmini.gpio_backlight, "gpio_backlight", 1))<0) {
            sprintf(errmsg, "cannot set BACKLIGHT to output (line %d)", dhmini.bl_pin);
            err++ ;
            break ;
        }

    } while (0) ;

    /* Handle errors */
    if (err) {
        fprintf(stderr, "%s\n", errmsg);
        if (dhmini.spi_fd) close(dhmini.spi_fd);
        if (dhmini.gpio_dc) gpiod_line_release(dhmini.gpio_dc);
        if (dhmini.gpio_chip) gpiod_chip_close(dhmini.gpio_chip);
        return -1;
    }

    /* Send ST7789 init instructions */
    i=0 ;
    while (1) {
        cmd = st7789_init_seq[i];
        if (cmd==ST_INVALID)
            break ;
        i++;
        len = st7789_init_seq[i];
        i++;
        /* Send command */
        dh_spi_write_8bit(SPI_CMD, cmd);
        if (len>0) {
            /* Send data */
            for (j=0 ; j<len ; j++) {
                dh_spi_write_8bit(SPI_DAT, st7789_init_seq[i+j]);
            }
            i+=len ;
        }
    }
    dhmini.frame_sz = dhmini.width * dhmini.height * sizeof(uint16_t);
    dhmini.frame    = malloc(dhmini.frame_sz);

    dh_clear();
    dh_display();

    atexit(dh_close);
    return 0;
}

void dh_putpix(
    uint16_t x,
    uint16_t y,
    uint16_t color)
{
    if (x<0 || x>=dhmini.width || y<0 || y>=dhmini.height)
        return ;
    dhmini.frame[y*dhmini.width + x] = color;
}

void dh_frame_set(uint16_t * buf)
{
    memcpy(dhmini.frame, buf, dhmini.frame_sz);
}

/*
 * Pack an RGB (24-bit) frame into the dhmini buffer (16-bit)
 * Pixels are expected as R/G/B (8/8/8)
 */
#define PACK_RGB(r,g,b) (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3)
void dh_frame_set_rgb(uint8_t * buf)
{
    int i ;
    for (i=0 ; i<dhmini.width * dhmini.height ; i++) {
        dhmini.frame[i] = PACK_RGB(buf[3*i], buf[3*i+1], buf[3*i+2]);
    }
}

void dh_fill(uint16_t color)
{
    int i ;
    for (i=0 ; i<(dhmini.width * dhmini.height) ; i++) {
        dhmini.frame[i] = color ;
    }
    dh_display();
}

void dh_line(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint16_t color)
{
  int16_t xerr=0,
          yerr=0,
          delta_x = x2 - x1,
          delta_y = y2 - y1,
          distance;
  uint16_t incx,
           incy,
           uRow = x1,
           uCol = y1;

    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else {
        incy = -1;
        delta_y = -delta_y;
    }

    (delta_x > delta_y) ? (distance = delta_x) : (distance = delta_y);
    for (uint16_t t = 0; t <= distance + 1; t++) {
        dh_putpix(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void dh_rectangle(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    uint16_t color,
    int filled)
{
    if (filled) {
        for (uint16_t i = y1; i < y2; i++)
            for (uint16_t j = x1; j < x2; j++)
                dh_putpix(j, i, color);
    } else {
        dh_line(x1, y1, x2, y1, color);
        dh_line(x1, y1, x1, y2, color);
        dh_line(x1, y2, x2, y2, color);
        dh_line(x2, y1, x2, y2, color);
    }
}

