#ifndef _DH_IMAGE_H_
#define _DH_IMAGE_H_

/*
 * Read an image from a file and display it.
 * Supported formats:
 * PNG as RGB 888 320x240 pixels
 * Color JPEG as RGB 888 320x240 pixels
 * Raw frame buffers sized 320x240 (16-bit)
 * Raw RGB-888 buffers sized 320x240
 *
 * Image file type is only recognized through file
 * extensions. Could certainly do better.
 *
 */
int dh_image(char * filename);

#endif
