/*
 *      rgb_yPbPr_conversion.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Interface for the rgb_yPbPr_conversion.c file. Functions in this file 
 *      are used to convert between unscaled rgb floats and Y, Pb, Pr floats
 */

#ifndef RGB_YPBPR_CONVERSION_INCLUDED
#define RGB_YPBPR_CONVERSION_INCLUDED

/* CS40 Libraries */
#include <pnm.h>

extern A2Methods_UArray2 compress_yPbPr_conversion(Pnm_ppm image);
extern Pnm_ppm decompress_yPbPr_conversion(A2Methods_UArray2 component_pixels);

#endif