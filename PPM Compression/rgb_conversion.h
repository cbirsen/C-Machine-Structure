/*
 *      rgb_conversion.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Interface for the rgb_conversion.c file. Functions in this file are
 *      used to convert between scaled rgb integers and unscaled rgb floats
 */

#ifndef RGB_CONVERSION_INCLUDED
#define RGB_CONVERSION_INCLUDED

/* CS40 Libraries */
#include <pnm.h>

extern void compress_rgb_conversion(Pnm_ppm image);
extern void decompress_rgb_conversion(Pnm_ppm image);

#endif