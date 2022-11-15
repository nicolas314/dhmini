DHMini
======

C driver for the Pimoroni Mini Display Hat on RPI Zero/Zero2.

This library allows controlling the [Pimoroni Display Hat Mini](https://shop.pimoroni.com/products/display-hat-mini) (hereafter: DHMini) from C programs.


![DHmini with fancy fonts](/dhmini1.jpg)

![DHmini with console fonts](/dhmini2.jpg)

# Compiling

Dependencies:

- Make and C compiler (duh)
- GPIO support handled by libgpiod-dev
- Truetype font support handled by libfreetype-dev
- PNG image display support handled by libpng-dev
- JPEG image display support handled by libjpeg-dev

On Raspberry Pi OS this can be set by installing all relevant packages.
To build C programs you need:

    sudo apt install build-essential

Add GPIOd, freetype, PNG, and JPEG support with:

    sudo apt install libgpiod-dev libfreetype-dev libpng-dev libjpeg-dev

Add some fancy fonts to play with. On Raspberry OS there are tons of font packages, try it fonts-aenigma for a good selection:

    sudo apt install fonts-aenigma

Build the library and demo programs:

    make

Test your screen with:

    bin/demo
    bin/banner Hello fonts/AbrilFatFace-Regular.ttf
    bin/banner Hello

# Usage

Initialize the library with `dh_init(NULL)`, close it with `dh_close()`.
`dh_close()` is also registered with `atexit()` in `dh_init()` so no need to call it if your program deliberately exits or returns from main.

# Assumptions

- Only one DHMini is connected.
- The Linux kernel attaches DHMini to spidev #1. If it attaches to a different SPI descriptor, this can be changed by passing a configuration struct to `dh_init()`.
- Byte-swapping is needed between Raspberry (Arm) byte ordering and ST7789 for 16-bit values. If this is not the case, the `SWAP16()` macro needs to be modified to not change its argument.
- Pointer size is the same as sizeof(unsigned long). This is true on 32-bit and 64-bit versions of Raspberry OS.

# Concepts

The library initializes an internal structure holding everything needed to communicate with the hat.
Accessor functions are provided to use the hat and read its current state.

The library exposes a frame buffer as a pointer to a width x height array of 16-bit pixels. Default width is 320 pixels, default height is 240 pixels, the screen orientation is such that the micro-USB connectors on a Raspberry Zero (or Zero 2) are up.


# Useful functions

See header files for more information.



