#ifndef _DHTEXT_H_
#define _DHTEXT_H_

/*
 * Display text with fancy TTF fonts
 * Arguments:
 * text: String containing text to print
 * fontname: Full path name to TTF font file
 * color: RGB triplet for text foreground
 * at_x, at_y: Start top-left position
 * size_x, size_y: Size of the text in pixels
 *
 * This function draws into the frame buffer.
 * Use dh_display() to show the results on screen.
 */

int dh_text(
    char * text,
    char * fontname,
    uint8_t color[3],
    int at_x,
    int at_y,
    int size_x,
    int size_y);

#endif
