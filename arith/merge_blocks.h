/*
 *      merge_blocks.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Interface for the merge_blocks.c file. Functions in this file are used
 *      to convert between component video values and quantized values
 */

#ifndef MERGE_BLOCKS_INCLUDED
#define MERGE_BLOCKS_INCLUDED

/* CS40 Libraries */
#include <a2methods.h>

extern A2Methods_UArray2 compress_merge(A2Methods_UArray2 component_pixels);
extern A2Methods_UArray2 decompress_merge_blocks(A2Methods_UArray2 quant_pixs);

#endif